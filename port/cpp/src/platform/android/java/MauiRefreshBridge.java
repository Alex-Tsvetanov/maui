// MauiRefreshBridge — the Java half of the pull-to-refresh seam.
//
// WHY A JAVA CLASS AT ALL. SwipeRefreshLayout delivers the completed pull through
// SwipeRefreshLayout.OnRefreshListener, a Java INTERFACE, and JNI cannot implement one: NewObject /
// CallVoidMethod only CALL Java, they cannot supply an implementation of a Java type. So the listener has
// to exist as real Java and the C++ side becomes the thing it calls — the same shape MauiDialogBridge
// (listener interfaces) and MauiItemsAdapter (an abstract class) already use, and this class deliberately
// copies their discipline rather than inventing a third one.
//
// PORTS: RefreshViewHandler.Android.cs:21 `platformView.Refresh += OnSwipeRefresh` and OnSwipeRefresh's
// body, `VirtualView.IsRefreshing = true` (lines 24-27). The native side spells that as
// refresh_view_handler::request_refresh(), which every backend already shares.
//
// THE PEER IS AN OPAQUE TOKEN, never a pointer Java dereferences. The native side resolves it through its
// live-peer registry (src/platform/android/android_refresh_ops.hpp), so a pull that lands after the owning
// handler has been torn down resolves to nothing and returns. A SwipeRefreshLayout is retained by the view
// tree and can outlive its handler, which is exactly the case the registry exists for.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library; the app hosts dex *.java from here, so no build wiring is needed. The
// androidx.swiperefreshlayout AAR is on the javac classpath via tools/parity/lib/android_aar_deps.txt.
package dev.mauicpp;

import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

public final class MauiRefreshBridge implements SwipeRefreshLayout.OnRefreshListener {
    private final long peer;

    public MauiRefreshBridge(long peer) {
        this.peer = peer;
    }

    // Fired once, on the UI thread, when a pull passes the trigger distance. SwipeRefreshLayout has
    // ALREADY put itself into the refreshing state by this point and keeps spinning until something calls
    // setRefreshing(false) — which the port drives from IsRefreshing, exactly like MAUI's MapIsRefreshing.
    @Override
    public void onRefresh() {
        nativeOnRefresh(peer);
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance is constructed. The peer is validated
    // native-side against the live-peer registry.
    private static native void nativeOnRefresh(long peer);
}
