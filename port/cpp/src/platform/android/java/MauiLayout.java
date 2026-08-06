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
// ARBITRARY-STROKESHAPE BORDER (dispatchDraw + canvas): a Border whose StrokeShape is NOT a shape the
// GradientDrawable background can express (a rounded rect / ellipse / plain rect — all handled by
// border_handler.cpp's GradientDrawable + Outline clip) needs a canvas draw to trace an arbitrary path
// (a Polygon triangle, a PathGeometry). This is the port twin of MAUI's Android Border render: MAUI's
// ContentViewGroup (a PlatformContentViewGroup) draws the MauiDrawable (fill + stroke tracing the shape
// path) as its background and clips its content in dispatchDraw via canvas.clipPath (see
// src/Core/AndroidNative/.../PlatformContentViewGroup.java getClipPath + dispatchDraw, and
// MauiDrawable.Android.cs). When the border handler installs a borderPeer (a border_platform*), this
// MauiLayout runs the same three-phase draw: fill the shape path BEHIND the children, clip the children
// to the (stroke-inset) shape path, then stroke the shape path ON TOP. A 0 borderPeer (every non-border
// host: content_page / layout / scroll_view, and a border whose shape IS GradientDrawable-expressible)
// leaves dispatchDraw as the plain ViewGroup default — no regression to the working cases.
package dev.mauicpp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Path;
import android.view.View;
import android.view.ViewGroup;

public final class MauiLayout extends ViewGroup {
    private long crossPlatformPeer;
    // The border_platform* whose arbitrary StrokeShape this host draws on the canvas, or 0 (the default —
    // no border draw, the plain ViewGroup dispatchDraw). Installed by border_handler.cpp's arrange_native
    // only for a Border whose shape needs the canvas (a Polygon / Path); cleared to 0 in ~border_platform
    // before the struct dies so a late dispatchDraw never dereferences a dangling pointer.
    private long borderPeer;

    public MauiLayout(Context context) {
        super(context);
        // Match MAUI's LayoutViewGroup default: do NOT clip children to the panel bounds. MAUI's
        // LayoutViewGroup.OnLayout sets ClipBounds = null whenever ClipsToBounds is false (the ILayout
        // default), so a child positioned outside the panel's own frame still renders. An Android ViewGroup
        // instead defaults clipChildren/clipToPadding to TRUE, which would hide any child the cross-platform
        // arrange places past the panel edge — e.g. a FlexLayout column whose Grow="1" body pushes the
        // trailing FOOTER a few dp below the panel bottom (a known flex measure quirk shared with C#: a grow
        // child measured to the full available height makes the outer flex slightly over-allocate). Clearing
        // the clip here is the port twin of MAUI's ClipBounds = null default; update_clips_to_bounds re-enables
        // clipping (setClipChildren(true)) for a layout that explicitly sets ClipsToBounds = true.
        setClipChildren(false);
        setClipToPadding(false);
    }

    // The handler installs the peer (the layout_handler address) after construction and clears it (0) on
    // disconnect, before the native handler is destroyed.
    public void setCrossPlatformPeer(long peer) {
        this.crossPlatformPeer = peer;
    }

    // The border handler installs the border_platform* here (for an arbitrary-StrokeShape Border) and
    // clears it (0) in ~border_platform before the struct dies. Invalidate so the canvas draw refreshes
    // when the shape / stroke / fill changes (the handler calls setBorderPeer again from arrange_native).
    public void setBorderPeer(long peer) {
        this.borderPeer = peer;
        invalidate();
    }

    // Draw an arbitrary-StrokeShape Border (a Polygon / Path the GradientDrawable cannot trace): fill the
    // shape path BEHIND the children, clip the children to the stroke-inset shape path, then stroke the
    // shape outline ON TOP — the port twin of MAUI's MauiDrawable draw + ContentViewGroup.dispatchDraw
    // clip. A 0 borderPeer is the plain ViewGroup default (every non-arbitrary-border host).
    @Override
    protected void dispatchDraw(Canvas canvas) {
        final long peer = borderPeer;
        if (peer == 0L) {
            super.dispatchDraw(canvas);
            return;
        }
        final int w = canvas.getWidth();
        final int h = canvas.getHeight();
        // Phase 1 — fill the shape path behind the content. Each native draw builds an android_canvas that
        // applies canvas.scale(density) to map its point-space geometry to pixels; that scale must NOT leak
        // into the child draw or the next phase, so every native call is bracketed by save()/restore().
        final int fillMark = canvas.save();
        nativeDrawBorderFill(peer, canvas, w, h);
        canvas.restoreToCount(fillMark);
        // Phase 2 — clip the content to the (stroke-inset) inner shape path, then draw the children. save/
        // restore so the clip never leaks into the on-top stroke draw or a sibling's draw pass.
        final int checkpoint = canvas.save();
        final Path clip = nativeBorderClipPath(peer, w, h);
        if (clip != null && !clip.isEmpty()) {
            canvas.clipPath(clip);
        }
        super.dispatchDraw(canvas);
        canvas.restoreToCount(checkpoint);
        // Phase 3 — stroke the shape outline on top of the content (unclipped, so the full stroke shows).
        final int strokeMark = canvas.save();
        nativeDrawBorderStroke(peer, canvas, w, h);
        canvas.restoreToCount(strokeMark);
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

    // Report this host's size without measuring children (maui already measured/arranged them, and the
    // container handler frames this ViewGroup Exactly through platform_arrange) — but under an
    // UNSPECIFIED spec report the CONTENT extent: the union of the children's already-laid-out
    // right/bottom edges.
    //
    // The former body was resolveSize(0, spec) with a comment claiming it "never reports 0". That is true
    // for EXACTLY only: View.resolveSize returns `size` (here 0) for both UNSPECIFIED and AT_MOST. It went
    // unnoticed because platform_arrange always measures this host EXACTLY — except inside a scroller.
    // android.widget.ScrollView.measureChild ALWAYS measures its single document child with an UNSPECIFIED
    // height spec (that is how content may exceed the viewport and scroll), IGNORING its LayoutParams, so a
    // MauiLayout content host reported height 0, the scroller computed scrollRange = 0, and the page could
    // not be scrolled AT ALL. Measured on emulator-5554 before this change: the port's `clip` page renders
    // identically to MAUI at rest, its ScrollView reports scrollable="false" in the accessibility tree, and
    // an `input swipe` that moves MAUI's page by 96.14% of the frame moved the port's by 0 px.
    //
    // The children keep drawing at their absolute frames either way (onLayout is the handler's, not this
    // ViewGroup's), which is why the bug was invisible to every still comparison: only the scroll RANGE was
    // wrong, never the pixels at rest.
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(resolveContent(contentExtent(this, false), widthMeasureSpec),
                             resolveContent(contentExtent(this, true), heightMeasureSpec));
    }

    // The union of `group`'s laid-out children's right (or bottom) edges — the content extent a scroller
    // needs to know how far it may scroll. Shared with MauiCollectionContent, which hosts the same
    // absolutely-framed children under the same scroller.
    static int contentExtent(ViewGroup group, boolean vertical) {
        int extent = 0;
        for (int i = 0, n = group.getChildCount(); i < n; i++) {
            final View child = group.getChildAt(i);
            if (child.getVisibility() == GONE) {
                continue;
            }
            extent = Math.max(extent, vertical ? child.getBottom() : child.getRight());
        }
        return extent;
    }

    // EXACTLY → the spec size; AT_MOST → min(content, spec); UNSPECIFIED → the content extent.
    static int resolveContent(int content, int spec) {
        final int mode = MeasureSpec.getMode(spec);
        final int size = MeasureSpec.getSize(spec);
        if (mode == MeasureSpec.EXACTLY) {
            return size;
        }
        if (mode == MeasureSpec.AT_MOST) {
            return Math.min(content, size);
        }
        return content; // UNSPECIFIED
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance lays out (layout_handler.cpp's android
    // partial). An INSTANCE native so JNI hands the native side this View (for its display density, to
    // convert the pixel size to dp). The peer is the child's layout_handler; the native side resolves its
    // cross-platform layout and arranges the children into the given 0-origin size. Valid while the peer is
    // installed (the handler clears it on disconnect).
    private native void nativeArrange(long peer, int widthPx, int heightPx);

    // The arbitrary-StrokeShape border draw callbacks, bound from C++ via RegisterNatives in
    // border_handler.cpp (the same reflection-free binding nativeArrange / MauiShapeView.nativeDraw use —
    // no Java_* export symbol). The peer is a border_platform*; width/height are the canvas PIXEL size.
    //   nativeDrawBorderFill  — fill the shape path with the Border's background paint (behind children).
    //   nativeBorderClipPath  — build the stroke-inset inner shape Path (PIXEL coords) to clip children;
    //                           null / empty = no clip (a square/degenerate border).
    //   nativeDrawBorderStroke— stroke the shape outline with the Border's stroke paint (over children).
    private native void nativeDrawBorderFill(long peer, Canvas canvas, int widthPx, int heightPx);
    private native Path nativeBorderClipPath(long peer, int widthPx, int heightPx);
    private native void nativeDrawBorderStroke(long peer, Canvas canvas, int widthPx, int heightPx);
}
