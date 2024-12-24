/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.orbtv.orbservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class OrbService extends Service {
    private static final String TAG = OrbService.class.getSimpleName();

    private IBinder mBinder;
    public native IBinder createBinder();
    
    @Override
    public void onCreate() {
        super.onCreate();

        mBinder = createBinder();

        if (null == mBinder) {
            Log.w(TAG, "[java] Binder is null");
        }
        else {
            Log.d(TAG, "[java] Binder is ready");
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "[java] A client binds the service");

        return mBinder;
    }

    static {
        System.loadLibrary("org.orbtv.orbservice.native");
    }
}
