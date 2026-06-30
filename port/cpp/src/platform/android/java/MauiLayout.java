// dev.mauicpp.MauiLayout — the host ViewGroup for the Android backend's container handlers (layout /
// content_page / scroll_view content host). The C++ analog of MAUI's MauiPlatformView /
// LayoutViewGroup (Microsoft.Maui.Platform.LayoutViewGroup): a ViewGroup that hosts the arranged
// children and, on each system layout pass, re-runs the cross-platform arrange so the children land
// where maui's layout_manager places them.
//
// WHY onLayout RE-ARRANGES (the crux of the nested-layout fan-out): MAUI positions children through
// ICrossPlatformLayout.CrossPlatformArrange, and LayoutViewGroup.OnLayout calls it on EVERY layout
// traversal (see src/Core/src/Platform/Android/LayoutViewGroup.cs) with a 0-origin destination — it does
// NOT trust a one-shot frame to survive. Android fires a system layout traversal whenever anything in the
// subtree calls requestLayout() (addView, setText, every property push during boot), and a parent
// ViewGroup re-lays-out its children in onLayout each time. A no-op onLayout only "works" for a child the
// system happens to keep measured through some OTHER real ViewGroup above it (the window's FrameLayout
// force-lays-out the page root via MATCH_PARENT) — but a layout nested inside another MauiLayout has a
// no-op parent, so a late requestLayout() on it (PFLAG_FORCE_LAYOUT) is never satisfied and the whole
// nested subtree collapses / is not drawn. Re-running CrossPlatformArrange here re-lays-out every child to
// its maui-computed position on every pass, exactly as MAUI's LayoutViewGroup does, so nesting works.
//
// The cross-platform arrange runs CHILDREN-ONLY (it never re-frames this host — the host's own frame is
// owned by its handler's platform_arrange / its parent's arrange), and it is fed this view's LOCAL,
// 0-origin size: a native subview's frame is expressed in its superview's coordinate space, whose origin
// is (0,0). Carrying the absolute window origin here would double-offset every child. nativeArrange takes
// width/height in PIXELS; the native side divides by the display density to reach the dp the
// cross-platform layer speaks.
//
// onMeasure mirrors LayoutViewGroup.OnMeasure's degenerate case: a maui container computes its own size
// through its layout_manager (the control's measure/arrange, not the ViewGroup's), and the handler frames
// this ViewGroup EXACTLY via platform_arrange (measure Exactly + layout). So onMeasure just resolves the
// incoming spec and reports it — it must not try to measure children (maui already measured them) and must
// never report 0 (which would zero the panel and clip every child). resolveSize honours an Exactly/AtMost
// spec and falls back to the spec size for Unspecified.
//
// crossPlatformPeer is the address of the child's layout_handler; the handler installs it after
// construction (on_connect_handler) and clears it (setCrossPlatformPeer(0)) on disconnect, before the
// handler can die — so onLayout never dereferences a dangling pointer (a 0 peer makes onLayout a no-op,
// the VM-less / unwired degradation). nativeArrange is bound from C++ via RegisterNatives (the
// reflection-free MauiShapeView.nativeDraw recipe) — no Java_* export symbol is needed.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library: runtime classes the native backend needs in the process' dex. The widget test host
// and the gallery app host both dex every *.java here (tools/android-testhost-run.sh and
// tools/parity/build_android_apphost.sh glob this dir), so MauiLayout.java is picked up automatically
// alongside NativeOnClickListener.java — no script edits needed.
package dev.mauicpp;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;

public final class MauiLayout extends ViewGroup {
    private long crossPlatformPeer;

    public MauiLayout(Context context) {
        super(context);
    }

    // The handler installs the peer (the layout_handler address) after construction and clears it (0) on
    // disconnect, before the native handler is destroyed.
    public void setCrossPlatformPeer(long peer) {
        this.crossPlatformPeer = peer;
    }

    // Re-run the cross-platform arrange so the children are positioned where maui's layout_manager places
    // them, on every system layout traversal (LayoutViewGroup.OnLayout). The destination is this view's
    // LOCAL, 0-origin size in pixels (right-left, bottom-top); the native side converts to dp. A 0 peer
    // (VM-less / not yet wired) falls back to a no-op — the children then keep whatever frames their own
    // platform_arrange set (the previous behaviour, valid only for an un-nested page root).
    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        if (crossPlatformPeer != 0L) {
            nativeArrange(crossPlatformPeer, right - left, bottom - top);
        }
    }

    // Report the resolved spec size without measuring children (maui already measured/arranged them,
    // and the container handler frames this ViewGroup Exactly through platform_arrange). resolveSize
    // honours Exactly/AtMost and yields the spec size for Unspecified — never 0, which would clip the
    // hosted children.
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final int width = resolveSize(0, widthMeasureSpec);
        final int height = resolveSize(0, heightMeasureSpec);
        setMeasuredDimension(width, height);
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance lays out (layout_handler.cpp's android
    // partial). An INSTANCE native so JNI hands the native side this View (for its display density, to
    // convert the pixel size to dp). The peer is the child's layout_handler; the native side resolves its
    // cross-platform layout and arranges the children into the given 0-origin size. Valid while the peer is
    // installed (the handler clears it on disconnect).
    private native void nativeArrange(long peer, int widthPx, int heightPx);
}
