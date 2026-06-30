package dev.mauicpp.apphost;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

// MauiHostActivity — the minimal real-Activity host for the C++ MAUI gallery on Android (the
// real-Activity analog of the app_process widget test host's Bootstrap). Unlike the testhost (which
// runs gtest under app_process and so has no window), a real Activity gives the maui Window an actual
// on-screen content view — which is what unlocks ON-DEVICE VISUAL PARITY and the verification of the
// EditText/Switch/CheckBox controls that cannot even be constructed under the shell-uid app_process.
//
// onCreate loads the app-host native library, asks it to build + mount the maui page named by the
// MAUI_SAMPLE_PAGE intent extra (defaulting to "label"), and installs the returned root
// android.view.View as the Activity's content view — exactly the SetContentView(rootView) that
// src/platform/android/window_handler.cpp's host_content was built to feed (it already mounts the page
// into a FrameLayout). The capture pipeline launches this Activity per page key and `adb screencap`s it.
public final class MauiHostActivity extends Activity {
    static {
        // libmaui_android_apphost.so = the gallery page-builders + maui_controls + the JNI mount entry
        // (src/platform/android/apphost/app_host.cpp), packaged under lib/<abi>/ in the APK.
        System.loadLibrary("maui_android_apphost");
    }

    // Implemented in src/platform/android/apphost/app_host.cpp: pins the JavaVM + this Activity as the
    // process-wide app Context (set_java_vm / set_app_context), builds the gallery page for pageKey,
    // connects a maui window to it, and returns the window's content FrameLayout (or null on failure).
    private native View nativeMount(String pageKey);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String pageKey = getIntent() != null ? getIntent().getStringExtra("MAUI_SAMPLE_PAGE") : null;
        if (pageKey == null || pageKey.isEmpty()) {
            pageKey = "label";
        }
        View root = nativeMount(pageKey);
        if (root != null) {
            setContentView(root);
        }
    }
}
