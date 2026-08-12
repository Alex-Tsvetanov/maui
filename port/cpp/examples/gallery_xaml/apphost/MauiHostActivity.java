package dev.mauicpp.apphost.xaml;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

// MauiHostActivity — the C++&XAML twin of src/platform/android/apphost/MauiHostActivity.java. Identical
// shape (load the native lib, nativeMount the page named by the MAUI_SAMPLE_PAGE extra, setContentView the
// returned root View), but a DIFFERENT package (dev.mauicpp.apphost.xaml) so this XAML host coexists with
// the C++ builder host on the emulator — both APKs installed, captured back to back.
//
// The page is built from Views/<name>.xaml markup at COMPILE TIME (byte-literal codegen — the NDK's
// Clang 18 has no #embed) via the examples::Views::<name>_page() factory dispatch in
// examples/gallery_xaml/apphost/app_host.cpp. Same on-device visual-parity purpose as the C++ host, but
// for the C++&XAML column.
public final class MauiHostActivity extends Activity {
    static {
        // libmaui_android_apphost_xaml.so = the gallery_xaml page-factory TUs (bytes-mode) + maui_hosting +
        // the JNI mount entry (examples/gallery_xaml/apphost/app_host.cpp), packaged under lib/<abi>/.
        System.loadLibrary("maui_android_apphost_xaml");
    }

    // Implemented in examples/gallery_xaml/apphost/app_host.cpp: pins the JavaVM + this Activity as the
    // process-wide app Context, builds the gallery_xaml page for pageKey, connects a maui window to it, and
    // returns the window's content FrameLayout (or null on failure).
    private native View nativeMount(String pageKey, String appearance);

    // The window's FULL bounds + system-bar INSETS in PIXELS, {w, h, left, top, right, bottom} — see the C++
    // builder host's twin (src/platform/android/apphost/MauiHostActivity.java) and
    // src/platform/android/jni/host_layout_rects.hpp for why the two must stay separate rather than collapse
    // into one pre-inset height. null => the legacy DisplayMetrics - dimen-chrome single-rect fallback.
    // Called via JNI ("windowMetricsPx" "()[I").
    public int[] windowMetricsPx() {
        try {
            if (android.os.Build.VERSION.SDK_INT >= 30) {
                android.view.WindowMetrics wm = getWindowManager().getCurrentWindowMetrics();
                android.graphics.Insets bars = wm.getWindowInsets()
                    .getInsets(android.view.WindowInsets.Type.systemBars());
                return new int[] {
                    wm.getBounds().width(), wm.getBounds().height(),
                    bars.left, bars.top, bars.right, bars.bottom
                };
            }
        } catch (Throwable t) {
            // fall through to null -> native dimen fallback
        }
        return null;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Match MAUI's status-bar colorization (parity C1/C3): MauiReference's Maui.MainTheme sets
        // colorPrimaryDark = #BDBDBD (both themes) as android:statusBarColor — a light-gray bar with dark
        // icons. This bare host otherwise leaves the default ~#F0F0F3 bar. Set the color + light-status-bar
        // flag so the top bar matches MAUI in light AND dark (mirrors the C++ apphost's MauiHostActivity).
        // EDGE-TO-EDGE, as MAUI's net10.0-android host runs — the twin of the C++ builder host's call. The
        // content view must span the whole window for the per-view safe area (windowMetricsPx above) to mean
        // anything; setStatusBarColor still applies under it on API 30..34.
        if (android.os.Build.VERSION.SDK_INT >= 30) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        getWindow().setStatusBarColor(0xFFBDBDBD);
        android.view.View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
            decorView.getSystemUiVisibility() | android.view.View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        String pageKey = getIntent() != null ? getIntent().getStringExtra("MAUI_SAMPLE_PAGE") : null;
        if (pageKey == null || pageKey.isEmpty()) {
            pageKey = "value_controls";
        }
        // THE APP THEME COMES FROM THE DEVICE — Configuration.uiMode, exactly as AppInfo.RequestedTheme
        // reads it (AppInfo.android.cs:45-57) and exactly as the C++ apphost's MauiHostActivity now does.
        //
        // This read the MAUI_APPEARANCE intent extra until now, which is the same defect fixed on the C++
        // host: the extra paints the port's OWN surfaces (the root + window background below) while every
        // Android-theme-resolved default — text colour above all — keeps its light-mode value, so a dark
        // page rendered near-black text on a near-black background. It was masked here only because
        // build_android_apphost_xaml.sh sets DEVICE night mode *and* passes the extra, so the two sources
        // happened to agree; that script's comment already claimed this file read uiMode, and it did not.
        final int nightFlags = getResources().getConfiguration().uiMode
            & android.content.res.Configuration.UI_MODE_NIGHT_MASK;
        final String appearance =
            nightFlags == android.content.res.Configuration.UI_MODE_NIGHT_YES ? "dark" : "light";
        View root = nativeMount(pageKey, appearance);
        if (root != null) {
            // Theme-aware content-root surface (MAUI's page bg: white light / #121212 dark) so transparent
            // pages match MAUI; a page with an explicit Background paints over it. Mirrors the C++ apphost.
            int surface = "dark".equals(appearance) ? 0xFF121212 : 0xFFFFFFFF;
            root.setBackgroundColor(surface);
            // ALSO paint the WINDOW bg: a hardware WebView (SurfaceView) composites against the window bg
            // (default white), so WebView pages showed white in dark. Mirrors the C++ apphost fix.
            getWindow().setBackgroundDrawable(new android.graphics.drawable.ColorDrawable(surface));
            setContentView(root);
        }
    }
}
