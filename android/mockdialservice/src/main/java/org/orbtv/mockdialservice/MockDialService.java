package org.orbtv.mockdialservice;

import android.app.Service;
import android.content.Intent;
import android.net.MacAddress;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.util.LongSparseArray;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

/**
 * Implements a naive mock DIAL service for testing.
 *
 * Note: This service does not handle the used network configuration changing.
 */
public class MockDialService extends Service {
   private static final String TAG = MockDialService.class.getSimpleName();

   private static final String DIAL_FRIENDLY_NAME = "ORB Mock DIAL service";
   private static final String DIAL_MODEL_NAME = "ORB Mock";
   private static final String MAIN_ACTIVITY_UUID = "abcd-efgh-hijk-lmno";

   private static MockDialServiceImpl mBinder = null;
   private static String mMacAddress = "";
   private static String mHostAddress = "127.0.0.1";

   private static class MockDialServiceImpl extends IMockDialService.Stub {
      private static final int DIAL_STATUS_STOPPED = 0;
      private static final int DIAL_STATUS_HIDDEN = 1;
      private static final int DIAL_STATUS_RUNNING = 2;

      private final Object mLock = new Object();
      private int mNextId = 0;
      private final LongSparseArray<DialAppInfo> mDialApps = new LongSparseArray<>();

      @Override
      public String getHostAddress() throws RemoteException {
         return mHostAddress;
      }

      @Override
      public int registerApp(IMockDialServiceCallback callback, String name, String data1,
         String data2) throws RemoteException {
         synchronized (mLock) {
            if (jniRegisterApp(name, data1, data2)) {
               final int id = mNextId++;
               IBinder.DeathRecipient onCallbackDeath = () -> {
                  try {
                     unregisterApp(id);
                  } catch (RemoteException ignored) {
                  }
               };
               DialAppInfo dialApp = new DialAppInfo(name, callback);
               callback.asBinder().linkToDeath(onCallbackDeath, 0);
               mDialApps.append(id, dialApp);
               return id;
            }
         }
         return -1;
      }

      @Override
      public void unregisterApp(int id) throws RemoteException {
         synchronized (mLock) {
            final DialAppInfo dialApp = mDialApps.get(id);
            if (dialApp != null) {
               jniUnregisterApp(dialApp.name);
               mDialApps.remove(id);
            }
         }
      }

      protected int startApp(String name, String payload) {
         int status = DIAL_STATUS_STOPPED;
         synchronized (mLock) {
            final DialAppInfo dialApp = findDialApp(name);
            if (dialApp != null) {
               try {
                  status = dialApp.callback.startApp(payload);
               } catch (RemoteException e) {
                  e.printStackTrace();
               }
            }
         }
         return status;
      }

      protected int hideApp(String name) {
         int status = DIAL_STATUS_STOPPED;
         synchronized (mLock) {
            final DialAppInfo dialApp = findDialApp(name);
            if (dialApp != null) {
               try {
                  status = dialApp.callback.hideApp();
               } catch (RemoteException e) {
                  e.printStackTrace();
               }
            }
         }
         return status;
      }

      protected void stopApp(String name) {
         synchronized (mLock) {
            final DialAppInfo dialApp = findDialApp(name);
            if (dialApp != null) {
               try {
                  dialApp.callback.stopApp();
               } catch (RemoteException e) {
                  e.printStackTrace();
               }
            }
         }
      }

      protected int getAppStatus(String name) {
         int status = DIAL_STATUS_STOPPED;
         synchronized (mLock) {
            final DialAppInfo dialApp = findDialApp(name);
            if (dialApp != null) {
               try {
                  status = dialApp.callback.getAppStatus();
               } catch (RemoteException e) {
                  e.printStackTrace();
               }
            }
         }
         return status;
      }

      private DialAppInfo findDialApp(String name) {
         for (int i = 0, sz = mDialApps.size(); i < sz; i++) {
            final DialAppInfo dialApp = mDialApps.valueAt(i);
            if (dialApp.name.equals(name)) {
               return dialApp;
            }
         }
         return null;
      }

      private static class DialAppInfo {
         public String name;
         public IMockDialServiceCallback callback;

         public DialAppInfo(String name, IMockDialServiceCallback callback) {
            this.name = name;
            this.callback = callback;
         }
      }
   }

   @Override
   public void onCreate() {
      super.onCreate();
      if (!findNetworkConfiguration()) {
         Log.e(TAG, "Cannot find suitable network configuration");
         return;
      }
      if (!jniStartServer(MAIN_ACTIVITY_UUID, DIAL_FRIENDLY_NAME, DIAL_MODEL_NAME, mHostAddress,
         mMacAddress)) {
         Log.e(TAG, "Failed to start native server");
         return;
      }
      mBinder = new MockDialServiceImpl();
   }

   @Override
   public int onStartCommand(Intent intent, int flags, int startId) {
      return START_STICKY;
   }

   @Override
   public void onDestroy() {
      jniStopServer();
      super.onDestroy();
   }

   @Override
   public IBinder onBind(Intent intent) {
      return mBinder;
   }

   private static boolean findNetworkConfiguration() {
      try {
         NetworkInterface network;
         Enumeration<NetworkInterface> enumIf = NetworkInterface.getNetworkInterfaces();
         // Find a network that is: up, supports multicast and is not virtual
         while (enumIf != null && enumIf.hasMoreElements()) {
            network = enumIf.nextElement();
            try {
               if (network.isUp() && network.supportsMulticast() && !network.isVirtual()) {
                  InetAddress ip;
                  Enumeration<InetAddress> enumIp = network.getInetAddresses();
                  // Find an IP that is: IPv4 and not a loopback
                  while (enumIp.hasMoreElements()) {
                     ip = enumIp.nextElement();
                     if (!ip.isLoopbackAddress() && ip instanceof Inet4Address) {
                        mHostAddress = ip.getHostAddress();
                        byte[] macAddress = null;
                        try {
                           macAddress = network.getHardwareAddress();
                        } catch (SocketException ignored) {
                        }
                        if (macAddress != null) {
                           mMacAddress = MacAddress.fromBytes(macAddress).toOuiString();
                        } else {
                           // MAC address is inaccessible for non-system apps
                           mMacAddress = "";
                        }
                        return true;
                     }
                  }
               }
            } catch (SocketException ignored) {
            }
         }
      } catch (SocketException ignored) {
      }
      return false;
   }

   /**
    * Callbacks from the JNI interface
    */
   @SuppressWarnings("unused")
   private static int jniStartApp(String name, String payload) {
      return mBinder.startApp(name, payload);
   }

   @SuppressWarnings("unused")
   private static int jniHideApp(String name) {
      return mBinder.hideApp(name);
   }

   @SuppressWarnings("unused")
   private static void jniStopApp(String name) {
      mBinder.stopApp(name);
   }

   @SuppressWarnings("unused")
   private static int jniGetAppStatus(String name) {
      return mBinder.getAppStatus(name);
   }

   /**
    * Calls into the JNI interface
    */
   private static native boolean jniStartServer(String uuid, String friendlyName,
      String modelName, String ipAddr, String macAddr);
   private static native void jniStopServer();
   private static native boolean jniRegisterApp(String name, String data1, String data2);
   private static native void jniUnregisterApp(String name);

   static {
      System.loadLibrary("org.orbtv.mockdialservice.mockdialservice-jni");
   }
}
