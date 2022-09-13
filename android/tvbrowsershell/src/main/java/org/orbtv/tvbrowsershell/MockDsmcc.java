/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at the top level of this
 * repository.
 */

package org.orbtv.tvbrowsershell;

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import org.orbtv.tvbrowser.IDsmccClient;
import org.orbtv.tvbrowser.IDsmccEngine;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.HashMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


public class MockDsmcc implements IDsmccEngine {
   private static final String TAG = "MockDsmcc";

   private final Context mContext;
   private String mBasePath;
   private String mDsmPath = "";
   private IDsmccClient mDsmccClient;
   private final HashMap<Integer, Handler> mActiveSubscriptionList;

   MockDsmcc(Context context) {
      mContext = context;
      mBasePath = context.getDataDir().getPath() + "/dsmcc/";
      File base = new File(mBasePath);
      if (base.exists()) {
         Utils.recursiveDelete(base);
      }
      mActiveSubscriptionList = new HashMap<>();
   }

   public void setDsmccData(String dsmccData) {
      if (dsmccData.isEmpty()) {
         return;
      }
      mDsmPath = unpackMockDsmcc(mContext, dsmccData);
      Log.d(TAG, "Using path: " + mDsmPath);
   }

   /**
    * Called by TvBrowser library at initialisation
    *
    * @param client Client to return DSMCC files and stream events
    */
   public void setDsmccClient(IDsmccClient client) {
      mDsmccClient = client;
   }

   /**
    * Request file from DSM-CC
    *
    * @param url DVB Url of requested file
    * @param requestId ID of request (returned to DsmccClient.onReceiveContent)
    */
   public boolean requestDvbContent(String url, int requestId) {
      if (mDsmPath.isEmpty()) {
         return false;
      }
      Uri uri = Uri.parse(url);
      String path = mDsmPath + "/" + uri.getAuthority() + uri.getPath();
      Log.d(TAG, "Get mock content: " + path);
      File file = new File(path);
      if (file.exists()) {
         if (file.isDirectory()) {
            StringBuilder sb = new StringBuilder();
            File[] files = file.listFiles();
            if (files != null) {
               for (File child : files) {
                  if (sb.length() != 0) sb.append(",");
                  sb.append(child.getName());
               }
            }
            mDsmccClient.onReceiveContent(requestId, ByteBuffer.wrap(sb.toString().getBytes()));
            return true;
         } else {
            Path p = Paths.get(path);
            try {
               FileChannel channel = FileChannel.open(p, StandardOpenOption.READ);
               ByteBuffer buffer = ByteBuffer.allocate((int) channel.size());
               channel.read(buffer);
               buffer.flip();
               channel.close();
               mDsmccClient.onReceiveContent(requestId, buffer);
               return true;
            } catch (IOException e) {
               e.printStackTrace();
            }
         }
      }
      return false;
   }

   /**
    * Release resources for DSM-CC file request
    *
    * @param requestId ID of request
    */
   public void closeDvbContent(int requestId) {
   }

   Handler sendStreamEvents(String url, String name, int listenId, int componentTag) {
      Handler handler = new android.os.Handler(Looper.getMainLooper());
      handler.postDelayed(new Runnable() {
         private int count = 0;

         public void run() {
            Handler handler = mActiveSubscriptionList.get(listenId);
            if (handler != null) {
               String data, text, status;
               if (!url.endsWith("/xxx") && (componentTag != 29)) {
                  status = "trigger";
                  switch (count) {
                     case 0:
                        data = "48656c6c6f204862625456";
                        text = "Hello HbbTV";
                        break;
                     case 1:
                        data = "54657374206576656e7420c3a4c3b6c3bc21";
                        text = "Test event äöü!";
                        break;
                     case 2:
                     default:
                        data = "cafebabe0008090a0d101fff";
                        text = "\n\r";
                        break;
                  }
                  count++;
                  if (count != 3) {
                     handler.postDelayed(this, 2000);
                  }
               } else {
                  data = "";
                  text = "";
                  status = "error";
               }
               mDsmccClient.onReceiveStreamEvent(listenId, name, data, text, status);
            }
         }
      }, 2000);
      return handler;
   }

   /**
    * Subscribe to DSM-CC Stream Event with URL and event name
    *
    * @param url DVB Url of stream event object
    * @param name Name of stream event
    * @param listenId ID of subscriber
    */
   public boolean subscribeStreamEventName(String url, String name, int listenId) {
      Handler handler = sendStreamEvents(url, name, listenId, -1);
      mActiveSubscriptionList.put(listenId, handler);
      return true;
   }

   /**
    * Subscribe to DSM-CC Stream Event with component tag and event ID
    *
    * @param name Name of stream event
    * @param componentTag Component tag for stream event
    * @param eventId Event Id of stream event
    * @param listenId ID of subscriber
    */
   public boolean subscribeStreamEventId(String name, int componentTag, int eventId, int listenId) {
      Handler handler = sendStreamEvents("url", name, listenId, componentTag);
      mActiveSubscriptionList.put(listenId, handler);
      return true;
   }

   /**
    * Unsubscribe to DSM-CC Stream Event
    *
    * @param listenId ID of subscriber
    */
   public void unsubscribeStreamEvent(int listenId) {
      Handler handler = mActiveSubscriptionList.get(listenId);
      if (handler != null) {
         handler.removeCallbacksAndMessages(null);
      }
      mActiveSubscriptionList.remove(listenId);
   }

   private String unpackMockDsmcc(Context context, String dsmccData) {
      String path = mBasePath + dsmccData.replace("/", "_") + "/";
      File dsmdir = new File(path);
      if (!dsmdir.exists()) {
         Log.i(TAG, "Unpacking Mock Dsmcc to: " + path);
         try {
            InputStream is = context.getAssets().open("tests/" + dsmccData);
            ZipInputStream zis = new ZipInputStream(new BufferedInputStream(is));
            String filename;
            ZipEntry ze;
            byte[] buffer = new byte[1024];
            int count;
            Log.i(TAG, "unpackMockDsmcc zip avail: " + zis.available());

            while ((ze = zis.getNextEntry()) != null) {
               filename = ze.getName();
               Log.i(TAG, "Dsmcc file: " + filename);
               if (ze.isDirectory()) {
                  File fmd = new File(path + filename);
                  if (!fmd.mkdirs()) {
                     Log.e(TAG, "unpackMockDsmcc mkdirs FAILED: " + path + filename);
                     break;
                  }
               } else {
                  FileOutputStream fout = new FileOutputStream(path + filename);
                  while ((count = zis.read(buffer)) != -1) {
                     fout.write(buffer, 0, count);
                  }
                  fout.close();
                  zis.closeEntry();
               }
            }
            zis.close();
         } catch (IOException e) {
            e.printStackTrace();
         }
      }
      return dsmdir.getPath();
   }
}
