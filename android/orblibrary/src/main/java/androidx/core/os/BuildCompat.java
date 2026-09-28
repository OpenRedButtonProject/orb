package androidx.core.os;

/**
 * Stub for the two methods Cronet 119 calls. This process is Android 12, so both are false.
 */
public final class BuildCompat {
    private BuildCompat() {
    }

    public @interface PrereleaseSdkCheck {
    }

    @PrereleaseSdkCheck
    public static boolean isAtLeastT() {
        return false;
    }

    @PrereleaseSdkCheck
    public static boolean isAtLeastU() {
        return false;
    }
}
