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
import android.os.IBinder;
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
import com.amazonaws.services.transcribe.model.LanguageCode;
import com.amazonaws.services.transcribe.model.ListTranscriptionJobsRequest;
import com.amazonaws.services.transcribe.model.ListTranscriptionJobsResult;
import com.amazonaws.services.transcribe.model.Media;
import com.amazonaws.services.transcribe.model.StartTranscriptionJobRequest;
import com.amazonaws.services.transcribe.model.TranscriptionJobSummary;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class VoiceRecognitionService extends Service {
    private static final String TAG = VoiceRecognitionService.class.getSimpleName();
    private MediaRecorder mediaRecorder;
    private boolean isRecording = false;
    // AWS S3 and Transcribe credentials and configurations
    // Set up the s3 bucket with name same as BUCKET_NAME
    // Set up the IAM user with correct access s3 bucket
    private static final String AWS_ACCESS_KEY = "TODO"; //TODO
    private static final String AWS_SECRET_KEY = "TODO"; //TODO
    private static final String BUCKET_NAME = "TODO"; //TODO
    private static final String OBJECT_KEY = "TODO"; //TODO
    private static final com.amazonaws.regions.Region REGION = Region.EU_Ireland.toAWSRegion();
    private AmazonS3 s3;
    private AmazonTranscribe amazonTranscribe;
    private BasicAWSCredentials awsCredentials;
    private Context mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;
        Intent permissionCheck = new Intent(mContext, PermissionRequestActivity.class);
        permissionCheck.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(permissionCheck);
        awsCredentials = new BasicAWSCredentials(AWS_ACCESS_KEY, AWS_SECRET_KEY);
        s3 = new AmazonS3Client(awsCredentials, REGION);
        amazonTranscribe = new AmazonTranscribeClient(awsCredentials);
        amazonTranscribe.setRegion(REGION);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!isRecording) {
            if (checkPermissions()) {
                Log.d(TAG, "Recording...");
                startRecording();
            } else {
                Log.d(TAG, "No permission.");
                requestPermissions();
            }
        } else {
            stopRecording();
            Log.d(TAG, "Analysing... It may take few seconds.");
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
            mediaRecorder = new MediaRecorder();
            mediaRecorder.reset();
            mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.AMR_NB); // Set the output format to AMR
            mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB); // Set the audio encoder to AMR
            mediaRecorder.setOutputFile(getOutputFilePath()); // Set the output file path
            mediaRecorder.prepare();
            mediaRecorder.start();
            isRecording = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void stopRecording() {
        if (mediaRecorder != null && isRecording) {
            mediaRecorder.stop();
            mediaRecorder.release();
            mediaRecorder = null;
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
            // Create a DeleteObjectRequest with the bucket name and object key
            DeleteObjectRequest deleteObjectRequest = new DeleteObjectRequest(BUCKET_NAME, OBJECT_KEY);
            // Delete the object from S3
            s3.deleteObject(deleteObjectRequest);
            if (!fileToUpload.exists()) {
                Log.e(TAG, path + " does not exist!");
                return null;
            }

            TransferUtility transferUtility = TransferUtility
                    .builder()
                    .context(mContext.getApplicationContext())
                    .defaultBucket(BUCKET_NAME)
                    .s3Client(s3)
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
            startTranscriptionJobRequest.withMedia(media)
                    .withLanguageCode(LanguageCode.EnGB)
                    .withMediaFormat(fileName)
                    .withOutputBucketName(bucketName)
                    .withTranscriptionJobName(jobName);
            amazonTranscribe.startTranscriptionJob(startTranscriptionJobRequest);
            // Poll for the job status
            String status = "";
            while (!status.equalsIgnoreCase("COMPLETED")) {
                GetTranscriptionJobRequest request = new GetTranscriptionJobRequest();
                request.withTranscriptionJobName(jobName);
                GetTranscriptionJobResult jobResult = amazonTranscribe.getTranscriptionJob(request);
                status = jobResult.getTranscriptionJob().getTranscriptionJobStatus();
                if (status.equalsIgnoreCase("FAILED")) {
                    Log.e(TAG, jobName + " has failed");
                    Log.e(TAG, jobName + " - " + jobResult.getTranscriptionJob().getFailureReason());
                    break;
                } else if (status.equalsIgnoreCase("COMPLETED")) {
                    try {
                        // Download the JSON file from S3
                        S3Object s3Object = s3.getObject(bucketName, OBJECT_KEY);
                        BufferedReader reader = new BufferedReader(new InputStreamReader(s3Object.getObjectContent()));
                        // Read the JSON data
                        StringBuilder jsonContent = new StringBuilder();
                        String line;
                        while ((line = reader.readLine()) != null) {
                            jsonContent.append(line);
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
            }
            return null;
        }

        protected void onPostExecute(String transcript) {
            if (transcript != null) {
                Log.d(TAG, transcript);
            }
        }
    }

    private boolean checkTranscriptionJobExists(String jobName) {
        ListTranscriptionJobsRequest listRequest = new ListTranscriptionJobsRequest();
        ListTranscriptionJobsResult listResult = amazonTranscribe.listTranscriptionJobs(listRequest);
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
        amazonTranscribe.deleteTranscriptionJob(deleteRequest);
    }
}
