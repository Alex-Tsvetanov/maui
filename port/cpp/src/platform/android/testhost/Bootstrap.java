// maui-cpp Android widget TEST-HOST bootstrap (M-android milestone 3) — the app_process pattern
// that monkey/uiautomator use: `adb shell CLASSPATH=<this, dexed> app_process /system/bin
// dev.mauicpp.testhost.Bootstrap <test .so> [gtest args...]` starts a full ART runtime as the shell
// user, this main() mints a system Context via ActivityThread.systemMain() (reflection — the class
// is not in the public android.jar), wraps it in a themed ContextThemeWrapper, loads the native
// test library, and hands the Context + the remaining argv across JNI so the embedded gtest suite
// can construct and drive REAL android.widget views — no APK, no gradle, no installation.
// Run it through tools/android-testhost-run.sh (which compiles/dexes/stages this file on demand).
package dev.mauicpp.testhost;

import android.content.Context;
import android.view.ContextThemeWrapper;
import java.util.Arrays;

public final class Bootstrap {
    private Bootstrap() {}

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.err.println("usage: Bootstrap <absolute path to the native test .so> [gtest args...]");
            System.exit(64);
        }
        // ActivityThread's constructor binds its Handler to the current thread's Looper — prepare it
        // first, exactly like SystemServer.run() does before createSystemContext().
        android.os.Looper.prepareMainLooper();
        // Zygote-forked processes inherit the preloaded font map; a directly-started app_process
        // runtime does not, so Typeface.DEFAULT stays null and any TextView/Button constructor NPEs
        // inside Typeface.create (verified on API 34). Trigger the same preload Zygote performs
        // (hidden API, hence reflection — the uiautomator-style hosts do exactly this).
        Class.forName("android.graphics.Typeface")
                .getMethod("loadPreinstalledSystemFontMap").invoke(null);
        Object activityThread = Class.forName("android.app.ActivityThread")
                .getMethod("systemMain").invoke(null);
        Context system = (Context) activityThread.getClass()
                .getMethod("getSystemContext").invoke(activityThread);
        // Widget constructors resolve their default-style attributes (Button reads
        // R.attr.buttonStyle) from the context theme, and the raw system context has none applied —
        // wrap it the way a test instrumentation would, so the native tests receive a Context that
        // is directly usable for `new android.widget.<View>(context)`.
        Context themed = new ContextThemeWrapper(system, android.R.style.Theme_DeviceDefault);
        System.load(args[0]);
        System.exit(nativeRun(themed, Arrays.copyOfRange(args, 1, args.length)));
    }

    // Implemented by the staged test library (src/platform/android/testhost/test_host.cpp): pins
    // the JavaVM + this Context for the JNI seam headers, then runs the embedded gtest suite with
    // the given args and returns RUN_ALL_TESTS()'s exit code.
    private static native int nativeRun(Context context, String[] args);
}
