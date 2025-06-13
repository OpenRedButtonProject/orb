package org.orbtv.orbsession;

public class OrbSessionConstants {

    public static final String KEY_HOST_TOKEN = "hostToken";
    public static final String KEY_DISPLAY_ID = "displayId";
    public static final String KEY_WIDTH = "width";
    public static final String KEY_HEIGHT = "height";
    public static final String KEY_SURFACE_PACKAGE = "surfacePackage";

    public static class OrbSessionException extends Exception {
        public OrbSessionException(String message) {
            super(message);
        }
    }
}
