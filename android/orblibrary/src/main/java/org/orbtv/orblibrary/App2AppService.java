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

package org.orbtv.orblibrary;

class App2AppService {
    private static final String TAG = App2AppService.class.getSimpleName();

    public static App2AppService GetInstance() {
        if (singleton == null) {
            singleton = new App2AppService();
        }
        return singleton;
    }

    private App2AppService() {
    }

    public boolean Start(int localApp2AppPort, int remoteApp2AppPort) {
        return jniStart(localApp2AppPort, remoteApp2AppPort);
    }

    public void Stop() {
        jniStop();
    }

    private static App2AppService singleton = null;

    // Native interface
    private native boolean jniStart(int localApp2AppPort, int remoteApp2AppPort);

    private native void jniStop();
}

