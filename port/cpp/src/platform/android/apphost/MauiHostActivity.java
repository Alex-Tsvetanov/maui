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

    // The window's FULL bounds and its system-bar INSETS, in PIXELS, as
    // {width, height, insetLeft, insetTop, insetRight, insetBottom} — via the timing-safe
    // WindowManager.getCurrentWindowMetrics() (API 30+, valid at mount time, no view-attachment dependency).
    //
    // Both halves, kept SEPARATE on purpose. MAUI on net10.0-android is edge-to-edge and applies the safe
    // area per view (SafeAreaExtensions.ApplyAdjustedSafeAreaInsetsPx), so the native host needs the whole
    // window AND the insets and lets each view decide — a root Layout insets its children, a root Border
    // (SafeAreaEdges None) is centered on the WINDOW mid-line. This method used to return only the
    // pre-subtracted usable height, which collapsed the two and centred every root inside the shrunken
    // canvas; see src/platform/android/jni/host_layout_rects.hpp for the measurement.
    //
    // Returns null on older APIs / any failure, so the caller falls back to the legacy
    // DisplayMetrics - dimen-chrome single-rect path. Called via JNI ("windowMetricsPx" "()[I").
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
        // THE APP THEME COMES FROM THE DEVICE, exactly as it does for a real MAUI app.
        //
        // This used to read the MAUI_APPEARANCE intent extra and drive the theme from it. That was wrong,
        // and wrong in a way that only a dark capture could show: the extra painted the port's OWN surfaces
        // dark (root + window background below) while the ANDROID THEME stayed light, so every native
        // default that resolves from the theme kept its light-mode value. The visible result was
        // near-black text (8,8,8) on the near-black page (18,18,18) — measured across the board, e.g.
        // label/templated_view/formatted_text all paint the port's default text at (8,8,8) where MAUI
        // paints (184,184,184). label_handler.cpp is NOT at fault: it deliberately leaves the TextView's
        // theme ColorStateList untouched when TextColor is unset (mirroring MAUI's UpdateTextColor no-op
        // on null, and preserving disabled-state dimming). Its unstated assumption — that the Android
        // theme agrees with the app's appearance — is what an env-var-driven theme broke.
        //
        // Reading Configuration.uiMode fixes the whole class at once rather than per control: text,
        // status-bar icons, dividers, disabled states and every other theme-resolved default now follow
        // the same source. It is also what MAUI itself does, which is why setting device night mode was
        // what finally made the MAUI reference render dark.
        //
        // Capture implication: a dark capture must now set DEVICE night mode (`adb shell cmd uimode night
        // yes`) for the port exactly as it already does for the MAUI reference — one mechanism for both
        // columns instead of two that can disagree.
        final int nightFlags = getResources().getConfiguration().uiMode
            & android.content.res.Configuration.UI_MODE_NIGHT_MASK;
        final boolean isDark = nightFlags == android.content.res.Configuration.UI_MODE_NIGHT_YES;
        final String appearance = isDark ? "dark" : "light";

        // The status bar is DELIBERATELY light-gray in BOTH themes, and this is not an oversight.
        // MauiReference's Maui.MainTheme sets colorPrimaryDark = #BDBDBD and ships NO values-night, so
        // Android uses that as android:statusBarColor in dark mode too — MAUI genuinely renders a
        // light-gray bar with dark icons whatever the app theme is. Matching that is the parity target.
        //
        // I briefly "fixed" this to follow isDark, on the reasoning that light chrome in dark mode must be
        // a bug. It is not, and the change was a REGRESSION: it left label_dark at 6.33% diff, the entire
        // residual being the status-bar strip (MAUI 189,189,189 vs the port's 18,18,18). The comment that
        // was here already said "no values-night, so both themes" — it was correct and I overrode it.
        // Verify against captures/android/maui/*_dark.png before changing this again.
        // EDGE-TO-EDGE, exactly as MAUI's net10.0-android host runs. Without this the system FITS the content
        // view below the status bar and above the navigation bar, which pre-insets the canvas and makes the
        // per-view safe area (windowMetricsPx above) impossible to express — a root that declines the inset
        // still ended up inside the shrunken band. With it, the content view spans the whole window and
        // maui::hosting::drive_layout gets a real full/safe pair.
        //
        // The status-bar COLOR below is unaffected and still load-bearing: setStatusBarColor keeps working
        // under setDecorFitsSystemWindows(false) on API 30..34 (it is only ignored from API 35, where
        // edge-to-edge is enforced and the bar is forced transparent). MAUI renders the SAME light-gray bar
        // in both themes (Maui.MainTheme's colorPrimaryDark = #BDBDBD, no values-night), so if a future API
        // level drops this, the bar must be re-established some other way rather than left to the page
        // surface — a dark page showing through the top 136 px was a measured 6.33% regression on label_dark.
        if (android.os.Build.VERSION.SDK_INT >= 30) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        getWindow().setStatusBarColor(0xFFBDBDBD);
        android.view.View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
            decorView.getSystemUiVisibility() | android.view.View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);

        String pageKey = getIntent() != null ? getIntent().getStringExtra("MAUI_SAMPLE_PAGE") : null;
        if (pageKey == null || pageKey.isEmpty()) {
            pageKey = "label";
        }
        View root = nativeMount(pageKey, appearance);
        if (root != null) {
            // The port's ContentPage has no default background (transparent), so the window shows through.
            // MAUI's page surface follows the theme (white in light, #121212 dark). Paint the content root
            // with that theme surface so transparent pages match MAUI; a page with an explicit Background
            // still paints over it.
            int surface = "dark".equals(appearance) ? 0xFF121212 : 0xFFFFFFFF;
            root.setBackgroundColor(surface);
            // ALSO paint the WINDOW background: a hardware android.webkit.WebView (SurfaceView-backed) punches
            // through the view layer and composites against the WINDOW bg, which defaults to white — so on
            // WebView-hosting pages the dark #121212 root did not show (the web_view/hybrid_web_view dark
            // page-surface RED). Setting the window drawable makes the surface behind everything match the theme.
            getWindow().setBackgroundDrawable(new android.graphics.drawable.ColorDrawable(surface));
            setContentView(root);
        }
    }
}
