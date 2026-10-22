package androidx.core.os;

import android.os.Build;

/**
 * Replaces the AndroidX copy when the APK is built. That copy stops at
 * isAtLeastT(), and Cronet 119 also calls isAtLeastU(). On Android 12 that
 * call must exist and return false.
 */
public class BuildCompat {
    private BuildCompat() {
    }

    public @interface PrereleaseSdkCheck {
    }

    public static boolean isAtLeastN() {
        return Build.VERSION.SDK_INT >= 24;
    }

    public static boolean isAtLeastNMR1() {
        return Build.VERSION.SDK_INT >= 25;
    }

    public static boolean isAtLeastO() {
        return Build.VERSION.SDK_INT >= 26;
    }

    public static boolean isAtLeastOMR1() {
        return Build.VERSION.SDK_INT >= 27;
    }

    public static boolean isAtLeastP() {
        return Build.VERSION.SDK_INT >= 28;
    }

    public static boolean isAtLeastQ() {
        return Build.VERSION.SDK_INT >= 29;
    }

    public static boolean isAtLeastR() {
        return Build.VERSION.SDK_INT >= 30;
    }

    @PrereleaseSdkCheck
    public static boolean isAtLeastS() {
        return Build.VERSION.SDK_INT >= 31;
    }

    @PrereleaseSdkCheck
    public static boolean isAtLeastT() {
        return Build.VERSION.SDK_INT >= 33;
    }

    @PrereleaseSdkCheck
    public static boolean isAtLeastU() {
        return Build.VERSION.SDK_INT >= 34;
    }
}
