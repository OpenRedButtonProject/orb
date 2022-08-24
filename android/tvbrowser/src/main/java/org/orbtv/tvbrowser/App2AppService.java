/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

public class App2AppService {

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

