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
    private native View nativeMount(String pageKey, String appearance);

    // The window's USABLE CONTENT height in PIXELS = getCurrentWindowMetrics().getBounds().height() minus the
    // system-bar insets (status bar top + navigation/gesture bar bottom), via the timing-safe
    // WindowManager.getCurrentWindowMetrics() (API 30+, valid at mount time — no view-attachment dependency).
    // This is exactly the area MAUI lays its content into. The native display_size uses it DIRECTLY instead of
    // DisplayMetrics.heightPixels - navigation_bar_height dimen, which double-subtracted the chrome on API 30+
    // (DisplayMetrics.heightPixels already excludes the bars there), leaving *-row / auto-sized pages ~200px
    // short of the gesture-nav pill the real MAUI app reaches. Returns 0 on older APIs / any failure, so the
    // caller falls back to the legacy DisplayMetrics - dimen-chrome path. Called via JNI ("usableContentHeightPx" "()I").
    public int usableContentHeightPx() {
        try {
            if (android.os.Build.VERSION.SDK_INT >= 30) {
                android.view.WindowMetrics wm = getWindowManager().getCurrentWindowMetrics();
                android.graphics.Insets bars = wm.getWindowInsets()
                    .getInsets(android.view.WindowInsets.Type.systemBars());
                return wm.getBounds().height() - bars.top - bars.bottom;
            }
        } catch (Throwable t) {
            // fall through to 0 -> native dimen fallback
        }
        return 0;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Match MAUI's status-bar colorization for a pixel-accurate top bar (parity C1/C3): MauiReference's
        // Maui.MainTheme sets colorPrimaryDark = #BDBDBD (no values-night, so both themes), which Android uses
        // as android:statusBarColor — a light-gray bar with dark icons. This bare app_process host otherwise
        // leaves the default white/translucent bar (~#F0F0F3). Set both the color and the light-status-bar
        // flag (dark icons over the light bar) so the top bar matches MAUI in light AND dark.
        getWindow().setStatusBarColor(0xFFBDBDBD);
        android.view.View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
            decorView.getSystemUiVisibility() | android.view.View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        String pageKey = getIntent() != null ? getIntent().getStringExtra("MAUI_SAMPLE_PAGE") : null;
        if (pageKey == null || pageKey.isEmpty()) {
            pageKey = "label";
        }
        // MAUI_APPEARANCE=light|dark drives the app theme (AppThemeBinding). `am start` cannot set the
        // process env, so forward the intent extra into the native call (getenv is empty under a launch).
        String appearance = getIntent() != null ? getIntent().getStringExtra("MAUI_APPEARANCE") : null;
        if (appearance == null) {
            appearance = "light";
        }
        View root = nativeMount(pageKey, appearance);
        if (root != null) {
            // The port's ContentPage has no default background (transparent), so the window shows through.
            // MAUI's page surface follows the theme (white in light, #121212 dark). Paint the content root
            // with that theme surface so transparent pages match MAUI; a page with an explicit Background
            // still paints over it.
            root.setBackgroundColor("dark".equals(appearance) ? 0xFF121212 : 0xFFFFFFFF);
            setContentView(root);
        }
    }
}
