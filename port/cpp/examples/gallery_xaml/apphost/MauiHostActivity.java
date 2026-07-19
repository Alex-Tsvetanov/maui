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
    private native View nativeMount(String pageKey);

    // The window's USABLE CONTENT height in PIXELS = getCurrentWindowMetrics().getBounds().height() minus the
    // system-bar insets — see the C++ builder host's twin (src/platform/android/apphost/MauiHostActivity.java)
    // for the full rationale (DisplayMetrics.heightPixels double-subtract on API 30+). The native display_size
    // uses this DIRECTLY. 0 => the legacy DisplayMetrics - dimen-chrome fallback. Called via JNI ("usableContentHeightPx" "()I").
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
        String pageKey = getIntent() != null ? getIntent().getStringExtra("MAUI_SAMPLE_PAGE") : null;
        if (pageKey == null || pageKey.isEmpty()) {
            pageKey = "value_controls";
        }
        View root = nativeMount(pageKey);
        if (root != null) {
            setContentView(root);
        }
    }
}
