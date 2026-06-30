// dev.mauicpp.MauiLayout — the host ViewGroup for the Android backend's container handlers (layout /
// content_page / scroll_view content host). The C++ analog of MAUI's MauiPlatformView /
// LayoutViewGroup (Microsoft.Maui.Platform.LayoutViewGroup): a ViewGroup that ONLY hosts — it never
// positions its own children.
//
// WHY onLayout IS A NO-OP (the crux of the whole container fan-out): MAUI positions children
// ABSOLUTELY. Each control's handler::platform_arrange calls the child View's own
// View.layout(left, top, right, bottom) directly (see src/platform/android/button_handler.cpp's
// platform_arrange — measure Exactly + layout). A standard ViewGroup re-lays-out its children in
// onLayout on every layout pass, OVERRIDING the frames maui already set, so everything would collapse
// to the top-left. By making onLayout a no-op the children keep exactly the positions maui's
// layout_manager arranged them into — the same role AppKit's plain NSView container plays for the
// apple backend (src/platform/apple/layout_handler.mm: "the panel only HOSTS").
//
// onMeasure mirrors LayoutViewGroup.OnMeasure's degenerate case: a maui container computes its own
// size through its layout_manager (the control's measure/arrange, not the ViewGroup's), and the
// handler frames this ViewGroup EXACTLY via platform_arrange (measure Exactly + layout). So onMeasure
// just resolves the incoming spec and reports it — it must not try to measure children (maui already
// measured them) and must never report 0 (which would zero the panel and clip every child). The
// resolveSize call honours an Exactly/AtMost spec and falls back to the spec size for Unspecified.
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
    public MauiLayout(Context context) {
        super(context);
    }

    // No-op: children are positioned by maui's layout_manager via each child handler's
    // platform_arrange (child.layout(l,t,r,b)). Re-laying-out here would override those absolute
    // frames and collapse the page. (LayoutViewGroup hosts; maui arranges.)
    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        // intentionally empty
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
}
