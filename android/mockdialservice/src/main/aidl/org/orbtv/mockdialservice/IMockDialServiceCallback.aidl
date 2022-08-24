package org.orbtv.mockdialservice;

interface IMockDialServiceCallback {
   int startApp(String payload);
   int hideApp();
   void stopApp();
   int getAppStatus();
}
