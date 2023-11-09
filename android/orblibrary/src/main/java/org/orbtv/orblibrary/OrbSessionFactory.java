/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.orblibrary;

import android.content.Context;
import android.os.Handler;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.UUID;

public class OrbSessionFactory {
    private static final String TAG = OrbSessionFactory.class.getSimpleName();

    /**
     * Create a TV browser session.
     *
     * @param context The context of the application.
     * @param callback The implementation of TvBrowserCallback to call.
     * @param configuration Configuration for this session
     * @return A TV browser session.
     */
    public static IOrbSession createSession(Context context, IOrbSessionCallback callback,
                                            Configuration configuration) {
        OrbSession session = new OrbSession(context, callback, configuration);
        Handler handler = new Handler(context.getMainLooper());
        handler.post(() -> callback.onSessionReady(session));
        return session;
    }

    /**
     * Create a distinctive identifier. See HbbTV clause 12.1.5 for requirements.
     *
     * @param deviceUniqueValue A value unique to the device (e.g. serial number).
     * @param commonSecretValue A secret value common to the manufacturer, model or product family.
     * @param origin The origin of the HTML document (see HbbTV clause 6.3.2).
     * @return A distinctive identifier on success, null otherwise.
     */
    public static String createDistinctiveIdentifier(String deviceUniqueValue,
                                                     String commonSecretValue, String origin) {
        MessageDigest md;
        try {
            md = MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
        String changingValue = UUID.randomUUID().toString();
        String message = deviceUniqueValue + commonSecretValue + origin + changingValue;
        StringBuilder sb = new StringBuilder();
        for (byte b : md.digest(message.getBytes())) {
            sb.append(Integer.toHexString((b & 0xFF) | 0x100).substring(1, 3));
        }
        return sb.toString();
    }

    public static class Configuration {
        public final int mediaSyncWcPort;
        public final int mediaSyncCiiPort;
        public final int mediaSyncTsPort;
        public final int app2appLocalPort;
        public final int app2appRemotePort;
        public final String mainActivityUuid;
        public final String userAgent;
        public final String sansSerifFontFamily;
        public final String fixedFontFamily;
        public final boolean doNotTrackEnabled;

        /**
         *
         * @param mediaSyncWcPort
         * @param mediaSyncCiiPort
         * @param mediaSyncTsPort
         * @param app2appLocalPort
         * @param app2appRemotePort
         * @param mainActivityUuid
         * @param userAgent
         * @param sansSerifFontFamily
         * @param fixedFontFamily
         * @param doNotTrackEnabled If the user has enabled Do Not Track (DNT).
         */
        public Configuration(int mediaSyncWcPort, int mediaSyncCiiPort, int mediaSyncTsPort,
                             int app2appLocalPort, int app2appRemotePort, String mainActivityUuid, String userAgent,
                             String sansSerifFontFamily, String fixedFontFamily,
                             boolean doNotTrackEnabled) {
            this.mediaSyncWcPort = mediaSyncWcPort;
            this.mediaSyncCiiPort = mediaSyncCiiPort;
            this.mediaSyncTsPort = mediaSyncTsPort;
            this.app2appLocalPort = app2appLocalPort;
            this.app2appRemotePort = app2appRemotePort;
            this.mainActivityUuid = mainActivityUuid;
            this.userAgent = userAgent;
            this.sansSerifFontFamily = sansSerifFontFamily;
            this.fixedFontFamily = fixedFontFamily;
            this.doNotTrackEnabled = doNotTrackEnabled;
        }
    }

    static {
        System.loadLibrary("org.orbtv.orblibrary.native");
    }
}
