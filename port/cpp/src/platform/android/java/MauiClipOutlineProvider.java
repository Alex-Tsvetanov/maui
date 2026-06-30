// dev.mauicpp.MauiClipOutlineProvider — a ViewOutlineProvider that clips an arbitrary CONVEX
// android.graphics.Path. This is the generic-view twin of how MAUI masks a view's layer for
// VisualElement.Clip (iOS: WrapperView.SetClip installs a CAShapeLayer mask on the view's layer — see
// src/platform/apple/apple_visual_ops.hpp apply_clip + src/platform/ios image clip). The Android backend
// carries no per-control WrapperView, so it cannot intercept each control's draw to clip an arbitrary
// (non-convex) Path the way the iOS CAShapeLayer mask does. What it CAN do, for ANY view (a stock
// android.widget.Button / EditText / Spinner / SearchView, or a MauiLayout ViewGroup) without subclassing
// it, is install a ViewOutlineProvider + setClipToOutline(true): the framework then clips the whole view
// (background, content, and — for a ViewGroup — its children) to the outline.
//
// CONVEX-ONLY (the honest constraint): Outline-based clipping only takes effect when the outline can be
// queried for a clip path, which the framework restricts to CONVEX outlines (Outline.canClip()). An
// EllipseGeometry / RectangleGeometry / RoundRectangleGeometry clip — the convex shapes — clips exactly;
// a non-convex PathGeometry or a multi-region GeometryGroup yields an outline whose canClip() is false, so
// the framework silently skips the clip (the view renders unclipped — no crash, graceful degradation). The
// native handler is responsible for that decision: it builds the Path, hands it here, then reads back
// hasClip() (which reflects Outline.canClip after getOutline ran) to know whether the clip actually
// applied, and can keep the headless mirror / log a deferral for the non-convex case. The arbitrary-path
// case (the iOS CAShapeLayer-mask generality) is future work: it needs a custom WrapperView ViewGroup that
// overrides dispatchDraw to canvas.clipPath() the way MauiImageView.onDraw / MauiShapeView do — a deeper
// port than this outline path, deferred here on purpose.
//
// WHY NOT setConvexPath: Outline.setConvexPath(Path) was deprecated in API 30 in favour of
// Outline.setPath(Path) (which carries the same convex requirement). compileSdk/emulator are API 34, so
// setPath is the current call; it throws IllegalArgumentException only on a self-intersecting path, never
// on a merely-non-convex-but-simple one (that just yields canClip()==false), so a try/catch keeps a
// pathological geometry from crashing the outline pass.
//
// The Path is in the view's own PIXEL coordinate space — the native handler builds it by resolving the
// clip geometry against the view's live bounds (in points) and scaling every coordinate by the display
// density, exactly as image_handler's build_clip_path does for the MauiImageView mask. The handler
// reinstalls a fresh provider on every clip change AND on every platform_arrange (the geometry is
// bounds-dependent — a resize must rebuild it), then calls View.invalidateOutline() so the framework
// re-queries getOutline.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s AndroidNative Java support
// library: the widget test host and the gallery app host both dex every *.java here, so this class is
// picked up automatically alongside MauiImageView/MauiShapeView/MauiLayout — no build-script edits.
package dev.mauicpp;

import android.graphics.Outline;
import android.graphics.Path;
import android.view.View;
import android.view.ViewOutlineProvider;

public final class MauiClipOutlineProvider extends ViewOutlineProvider {
    // The native-built clip Path (in the target View's pixel coordinate space). Never null for an installed
    // provider (the handler installs BACKGROUND / clears clipToOutline to remove the clip rather than
    // hand a null path).
    private final Path clipPath;

    // Set true by getOutline when the framework accepted the path as clippable (Outline.canClip()). The
    // native handler reads this back (hasClip) after setOutlineProvider+invalidateOutline force a getOutline
    // to decide whether the convex clip actually applied or must fall back to the headless mirror.
    private boolean clipApplied;

    public MauiClipOutlineProvider(Path path) {
        this.clipPath = path;
    }

    // Whether the most recent getOutline produced a clippable (convex) outline. Reflects the framework's
    // Outline.canClip() for the path this provider carries.
    public boolean hasClip() {
        return clipApplied;
    }

    @Override
    public void getOutline(View view, Outline outline) {
        clipApplied = false;
        if (clipPath == null || clipPath.isEmpty()) {
            return;
        }
        try {
            // Outline.setPath: clip the view to this path. Only a convex path becomes clippable; the
            // framework reports that via Outline.canClip() (false for a non-convex/empty outline, in which
            // case setClipToOutline(true) is a no-op and the view renders unclipped — the documented
            // graceful degradation). A self-intersecting path can throw; the catch keeps it from crashing
            // the outline pass.
            outline.setPath(clipPath);
            clipApplied = outline.canClip();
        } catch (IllegalArgumentException ex) {
            // Pathological geometry — leave the outline empty (no clip), matching the non-convex fallback.
            clipApplied = false;
        }
    }
}
