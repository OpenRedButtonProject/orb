package org.orbtv.mockdialservice;

import org.orbtv.mockdialservice.IMockDialServiceCallback;

interface IMockDialService {
   String getHostAddress();
   int registerApp(IMockDialServiceCallback callback, String name, String data1, String data2);
   void unregisterApp(int id);
}
