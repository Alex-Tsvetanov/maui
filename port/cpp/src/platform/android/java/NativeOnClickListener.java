// dev.mauicpp.NativeOnClickListener — the JNI click trampoline for the Android backend's handlers
// (the C++ analog of ButtonHandler.Android.cs's ButtonClickListener : Java.Lang.Object,
// View.IOnClickListener). A handler partial constructs one with the address of its platform struct
// as the peer and installs it via View.setOnClickListener; onClick crosses back into C++ through
// nativeOnClick, which the partial binds with RegisterNatives (reflection-free, no Java_* symbol
// export needed — see src/platform/android/button_handler.cpp).
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative
// Java support library: runtime classes the native backend needs in the process' dex. The widget
// test host dexes it alongside testhost/Bootstrap.java (tools/android-testhost-run.sh); a future
// real app host must ship the same classes.
package dev.mauicpp;

import android.view.View;

public final class NativeOnClickListener implements View.OnClickListener {
    private final long peer;

    public NativeOnClickListener(long peer) {
        this.peer = peer;
    }

    @Override
    public void onClick(View view) {
        nativeOnClick(peer);
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance is constructed; the peer is the
    // handler's platform struct, valid while the listener is installed (the partial uninstalls the
    // listener on disconnect, before the struct can die).
    private static native void nativeOnClick(long peer);
}
