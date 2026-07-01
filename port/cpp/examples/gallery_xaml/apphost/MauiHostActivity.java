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
