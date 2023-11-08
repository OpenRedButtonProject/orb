/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.voicerecognitionservice;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.MediaRecorder;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import androidx.core.content.ContextCompat;

import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.mobileconnectors.s3.transferutility.TransferListener;
import com.amazonaws.mobileconnectors.s3.transferutility.TransferObserver;
import com.amazonaws.mobileconnectors.s3.transferutility.TransferState;
import com.amazonaws.mobileconnectors.s3.transferutility.TransferUtility;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3Client;
import com.amazonaws.services.s3.model.DeleteObjectRequest;
import com.amazonaws.services.s3.model.Region;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.transcribe.AmazonTranscribe;
import com.amazonaws.services.transcribe.AmazonTranscribeClient;
import com.amazonaws.services.transcribe.model.DeleteTranscriptionJobRequest;
import com.amazonaws.services.transcribe.model.GetTranscriptionJobRequest;
import com.amazonaws.services.transcribe.model.GetTranscriptionJobResult;
import com.amazonaws.services.transcribe.model.GetVocabularyRequest;
import com.amazonaws.services.transcribe.model.LanguageCode;
import com.amazonaws.services.transcribe.model.ListTranscriptionJobsRequest;
import com.amazonaws.services.transcribe.model.ListTranscriptionJobsResult;
import com.amazonaws.services.transcribe.model.Media;
import com.amazonaws.services.transcribe.model.Settings;
import com.amazonaws.services.transcribe.model.StartTranscriptionJobRequest;
import com.amazonaws.services.transcribe.model.TranscriptionJobSummary;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class VoiceRecognitionService extends Service {
    private static final String TAG = VoiceRecognitionService.class.getSimpleName();
    private MediaRecorder mMediaRecorder;
    private CommandParser mParser;
    private boolean isRecording = false;
    private final int RECORDING_LIMIT = 10; // set recording limit be 10 seconds
    private boolean hasAWSKey = false;
    public static final int LOG_MESSAGE = 99;

    // AWS S3 and Transcribe credentials and configurations
    // Set up the s3 bucket with name same as BUCKET_NAME
    // Set up the IAM user with correct access s3 bucket
    private String AWS_ACCESS_KEY;
    private String AWS_SECRET_KEY;
    private String BUCKET_NAME;
    private String OBJECT_KEY;
    private String TRANSCRIBE_VOCABULARY;
    private com.amazonaws.regions.Region REGION;
    private AmazonS3 mS3;
    private AmazonTranscribe mAmazonTranscribe;
    private BasicAWSCredentials mAwsCredentials;
    private Context mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;
        Intent permissionCheck = new Intent(mContext, PermissionRequestActivity.class);
        permissionCheck.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(permissionCheck);
        hasAWSKey = getAwsConfiguration();
        if (!hasAWSKey) {
            broadcastMessage("AWS Configuration setting error.");
            return;
        } else {
            broadcastMessage("Press RECORD to start recording.");
        }

        mAwsCredentials = new BasicAWSCredentials(AWS_ACCESS_KEY, AWS_SECRET_KEY);
        mS3 = new AmazonS3Client(mAwsCredentials, REGION);
        mAmazonTranscribe = new AmazonTranscribeClient(mAwsCredentials);
        mAmazonTranscribe.setRegion(REGION);
        mParser = new CommandParser();
    }

    private boolean getAwsConfiguration() {
        try {
            InputStream inputStream = getAssets().open("awsconfiguration.json");
            byte[] buffer = new byte[inputStream.available()];
            inputStream.read(buffer);
            inputStream.close();
            String json = new String(buffer, "UTF-8");

            JSONObject AWSConfig = new JSONObject(json);
            AWS_ACCESS_KEY = AWSConfig.getString("AWS_ACCESS_KEY");
            AWS_SECRET_KEY = AWSConfig.getString("AWS_SECRET_KEY");
            BUCKET_NAME = AWSConfig.getString("BUCKET_NAME");
            OBJECT_KEY = AWSConfig.getString("OBJECT_KEY");
            TRANSCRIBE_VOCABULARY = AWSConfig.getString("TRANSCRIBE_VOCABULARY");
            REGION = Region.fromValue(AWSConfig.getString("REGION")).toAWSRegion();
        } catch (Exception e) {
            return false;
        }
        return true;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!hasAWSKey) {
            return START_STICKY;
        }
        if (!isRecording) {
            if (checkPermissions()) {
                broadcastMessage("Start recording...");
                startRecording();
            } else {
                broadcastMessage("Request permission...");
                requestPermissions();
            }
        } else {
            stopRecording();
            broadcastMessage("Analysing... It may take few seconds.");
            // Upload the recorded audio to AWS S3 in the background
            new S3UploadTask().execute(getOutputFilePath());
        }
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private boolean checkPermissions() {
        int recordPermission = ContextCompat
                .checkSelfPermission(mContext, Manifest.permission.RECORD_AUDIO);
        int writePermission = ContextCompat
                .checkSelfPermission(mContext, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        return recordPermission == PackageManager.PERMISSION_GRANTED
                && writePermission == PackageManager.PERMISSION_GRANTED;
    }

    private void requestPermissions() {
        Intent permissionCheck = new Intent(mContext, PermissionRequestActivity.class);
        permissionCheck.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(permissionCheck);
    }

    private void startRecording() {
        CharSequence name = "Permission";
        String description = "Asking for permission";
        int importance = NotificationManager.IMPORTANCE_DEFAULT;
        NotificationChannel channel = new NotificationChannel("PermissionChannel",
                name,
                importance);
        channel.setDescription(description);
        NotificationManager notificationManager =
                (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(channel);
        Intent intent = new Intent(this, NotificationReceiverActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(mContext,
                1337,
                intent,
                PendingIntent.FLAG_IMMUTABLE);
        Notification n = new Notification.Builder(this)
                .setContentTitle("Permission")
                .setContentText("recording")
                .setSmallIcon(R.drawable.logo)
                .setContentIntent(pendingIntent)
                .setChannelId("PermissionChannel")
                .setAutoCancel(true).build();
        startForeground(1337, n);
        notificationManager.notify(1337, n);
        try {
            mMediaRecorder = new MediaRecorder();
            mMediaRecorder.reset();
            mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.AMR_NB); // Set the output format to AMR
            mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB); // Set the audio encoder to AMR
            mMediaRecorder.setOutputFile(getOutputFilePath()); // Set the output file path
            mMediaRecorder.setMaxDuration(RECORDING_LIMIT * 1000);
            mMediaRecorder.prepare();
            mMediaRecorder.setOnInfoListener(new MediaRecorder.OnInfoListener() {
                @Override
                public void onInfo(MediaRecorder mr, int what, int extra) {
                    if (what == MediaRecorder.MEDIA_RECORDER_INFO_MAX_DURATION_REACHED) {
                        broadcastMessage("Stop recording, audio over " +
                                RECORDING_LIMIT + " seconds.");
                        broadcastMessage("Press RECORD key to start recording.");
                        stopRecording();
                    }
                }
            });
            mMediaRecorder.start();
            isRecording = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void parseIncomingCommand(String transcript) {
        CommandParser.Command action = mParser.parseIncomingCommand(transcript);
        Log.d(TAG, "Got an action : " + action.actId);
        broadcastMessage(action.actId, action.item, action.anchor, action.offset);
    }

    private void broadcastMessage(String info) {
        broadcastMessage(LOG_MESSAGE, info, null, null);
    }

    private void broadcastMessage(Integer actId, String info, String anchor, Integer offset) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                Intent result = new Intent();
                result.setAction("org.orbtv.voiceservice.message");
                result.putExtra("actId", actId);
                result.putExtra("info", info);
                result.putExtra("anchor", anchor);
                result.putExtra("offset", offset);
                sendBroadcast(result);
            }
        });
    }

    private void stopRecording() {
        if (mMediaRecorder != null && isRecording) {
            mMediaRecorder.stop();
            mMediaRecorder.release();
            mMediaRecorder = null;
            isRecording = false;
        }
    }

    private String getOutputFilePath() {
        return mContext
                .getApplicationContext()
                .getExternalFilesDir(null)
                .getAbsolutePath() + "/tmp.amr";
    }

    private class S3UploadTask extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... params) {
            String path = params[0];
            File fileToUpload = new File(path);
            String keyName = fileToUpload.getName();
            if (!fileToUpload.exists()) {
                Log.e(TAG, path + " does not exist!");
                return null;
            }
            try {
                DeleteObjectRequest deleteObjectRequest =
                        new DeleteObjectRequest(BUCKET_NAME, OBJECT_KEY);
                mS3.deleteObject(deleteObjectRequest);
            } catch (Exception e) {
                broadcastMessage("AWS credentials are invalid. " +
                        "Cannot upload audio to aws.");
                return null;
            }

            TransferUtility transferUtility = TransferUtility
                    .builder()
                    .context(mContext.getApplicationContext())
                    .defaultBucket(BUCKET_NAME)
                    .s3Client(mS3)
                    .build();
            TransferObserver observer = transferUtility.upload(keyName, fileToUpload);
            observer.setTransferListener(new TransferListener() {
                @Override
                public void onStateChanged(int id, TransferState state) {
                    if (state == TransferState.COMPLETED) {
                        // Create a transcription job in AWS Transcribe in the background
                        new TranscribeTask().execute(BUCKET_NAME,
                                "amr",
                                new File(path).getName(),
                                "s3://" + BUCKET_NAME + "/" + keyName);
                    }
                }

                public void onProgressChanged(int id, long bytesCurrent, long bytesTotal) {
                }

                public void onError(int id, Exception e) {
                    Log.d(TAG, "Error during uploading audio command");
                }
            });
            return "s3://" + BUCKET_NAME + "/" + keyName;
        }
    }

    private class TranscribeTask extends AsyncTask<String, Void, Void> {
        @Override
        protected Void doInBackground(String... params) {
            String bucketName = params[0];
            String fileName = params[1];
            String jobName = params[2];
            String s3Path = params[3];
            if (checkTranscriptionJobExists(jobName)) {
                deleteTranscriptionJob(jobName);
            }
            StartTranscriptionJobRequest startTranscriptionJobRequest = new StartTranscriptionJobRequest();
            Media media = new Media();
            media.setMediaFileUri(s3Path);
            try {
                mAmazonTranscribe.getVocabulary(new GetVocabularyRequest()
                        .withVocabularyName(TRANSCRIBE_VOCABULARY));
                startTranscriptionJobRequest.withMedia(media)
                        .withLanguageCode(LanguageCode.EnGB)
                        .withMediaFormat(fileName)
                        .withOutputBucketName(bucketName)
                        .withTranscriptionJobName(jobName)
                        .withSettings(new Settings().withVocabularyName(TRANSCRIBE_VOCABULARY));
            } catch (Exception e) {
                broadcastMessage("Vocabulary does not exist: " +
                        TRANSCRIBE_VOCABULARY +
                        " transcribe without custom vocabulary.");
                startTranscriptionJobRequest.withMedia(media)
                        .withLanguageCode(LanguageCode.EnGB)
                        .withMediaFormat(fileName)
                        .withOutputBucketName(bucketName)
                        .withTranscriptionJobName(jobName);
            }

            mAmazonTranscribe.startTranscriptionJob(startTranscriptionJobRequest);
            // Poll for the job status
            String status = "";
            while (true) {
                GetTranscriptionJobRequest request = new GetTranscriptionJobRequest().withTranscriptionJobName(jobName);
                GetTranscriptionJobResult jobResult = mAmazonTranscribe.getTranscriptionJob(request);
                status = jobResult.getTranscriptionJob().getTranscriptionJobStatus();
                if (status.equalsIgnoreCase("FAILED")) {
                    Log.e(TAG, jobName + " has failed");
                    Log.e(TAG, jobName + " - " + jobResult.getTranscriptionJob().getFailureReason());
                    break;
                }
                if (status.equalsIgnoreCase("COMPLETED")) {
                    break;
                }
            }
            if (status.equalsIgnoreCase("COMPLETED")) {
                try {
                    // Download the JSON file from S3
                    S3Object s3Object = mS3.getObject(bucketName, OBJECT_KEY);
                    BufferedReader reader = new BufferedReader(new InputStreamReader(s3Object.getObjectContent()));
                    // Read the JSON data
                    StringBuilder jsonContent = new StringBuilder();
                    String currentLine;
                    while ((currentLine = reader.readLine()) != null) {
                        jsonContent.append(currentLine);
                    }
                    // Parse the JSON data using org.json
                    JSONObject jsonObject = new JSONObject(jsonContent.toString());
                    // Extract the transcript
                    JSONObject results = jsonObject.getJSONObject("results");
                    JSONArray transcripts = results.getJSONArray("transcripts");
                    JSONObject transcriptObject = transcripts.getJSONObject(0);
                    String transcript = transcriptObject.getString("transcript");
                    System.out.println("Transcript: " + transcript);
                    onPostExecute(transcript);
                    // Close resources
                    reader.close();
                    s3Object.close();
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (JSONException e) {
                    throw new RuntimeException(e);
                }
            }
            return null;
        }

        protected void onPostExecute(String transcript) {
            Log.d(TAG, "Transcript : " + transcript);
            if (transcript == null) {
                Log.e(TAG, "Error is found in audio command");
            } else if (transcript.isEmpty()) {
                broadcastMessage("Receive an empty audio command");
            } else {
                broadcastMessage("Receive audio command: " + transcript);
                parseIncomingCommand(transcript);
            }
        }
    }

    private boolean checkTranscriptionJobExists(String jobName) {
        ListTranscriptionJobsRequest listRequest = new ListTranscriptionJobsRequest();
        ListTranscriptionJobsResult listResult = mAmazonTranscribe.listTranscriptionJobs(listRequest);
        for (TranscriptionJobSummary transcriptionJob : listResult.getTranscriptionJobSummaries()) {
            if (transcriptionJob.getTranscriptionJobName().equals(jobName)) {
                return true;
            }
        }
        return false;
    }

    // Helper method to delete a Transcribe job
    private void deleteTranscriptionJob(String jobName) {
        DeleteTranscriptionJobRequest deleteRequest = new DeleteTranscriptionJobRequest();
        deleteRequest.withTranscriptionJobName(jobName);
        mAmazonTranscribe.deleteTranscriptionJob(deleteRequest);
    }
}
