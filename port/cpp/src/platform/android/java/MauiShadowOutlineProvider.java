// dev.mauicpp.MauiShadowOutlineProvider — a ViewOutlineProvider that defines the SHADOW-CASTER shape for
// a view's native elevation shadow. This is the Android-native analog of how MAUI draws a view's
// IView.Shadow. In real .NET MAUI on Android, ViewExtensions.UpdateShadow sets WrapperView.Shadow, and the
// AndroidNative PlatformWrapperView draws the shadow entirely in SOFTWARE: it takes the child's alpha mask,
// blurs it (BlurMaskFilter of Shadow.Radius), tints it (Shadow.Color × Shadow.Opacity), offsets it by
// Shadow.Offset, and composites it beneath the content in dispatchDraw (src/Core/AndroidNative/.../
// PlatformWrapperView.java). That software path faithfully supports Color, Radius, Offset(x,y) and Opacity.
//
// This AAR-less backend carries no per-control WrapperView, so it cannot intercept each control's draw to
// paint a software blurred shadow the way PlatformWrapperView does. Instead it uses the NATIVE elevation
// shadow the platform composites for free (RenderNode shadow):
//   - setElevation(z) on the view gives it a resting Z, and the framework composites a drop shadow whose
//     size/blur/spread scale with z;
//   - setOutlineProvider(this) defines the caster SILHOUETTE — a rounded-rect matching the view's bounds and
//     corner radius, via Outline.setRoundRect (always shadow-castable, unlike an arbitrary Outline.setPath);
//   - the view keeps setClipToOutline(FALSE) — the outline is used ONLY to shape the shadow, never to clip
//     the content (a shadow does not require, and here must not cause, content clipping);
//   - the native handler tints the shadow via View.setOutlineSpotShadowColor / setOutlineAmbientShadowColor
//     (API 28+) so the shadow renders in Shadow.Color at Shadow.Opacity — the COLORED glow (e.g. the red
//     glow the shadow_playground / invalidate_shadow_host pages expect), not just a gray Material shadow.
//
// DOCUMENTED DEVIATIONS from the C# WrapperView software shadow (each an API limitation of the native
// elevation-shadow path, not a behavior guess — the native colored shadow is a faithful APPROXIMATION of
// the visible result, chosen over shipping nothing):
//   - OFFSET: the native elevation shadow's position is derived by the platform from the view's Z and its
//     position relative to the system light source; there is NO per-view Shadow.Offset(x,y) API. The shadow
//     therefore falls in the platform's fixed direction (down/outward) rather than at Shadow.Offset. The
//     handler folds Offset only into the elevation magnitude (a larger offset → a slightly larger Z so the
//     shadow reads as "further"), so the GLOW is present and colored but its X/Y direction is the native
//     default, not the exact requested offset. (The exact arbitrary-offset colored shadow needs the software
//     WrapperView port.)
//   - RADIUS/BLUR: elevation Z is not the same unit as Shadow.Radius (a Gaussian blur radius). The handler
//     maps Radius→a proportional elevation so a larger radius reads as a larger, softer shadow, but the blur
//     falloff is the platform's, not an exact BlurMaskFilter of Radius.
//   - The shape is a ROUNDED RECT (bounds + corner radius). An arbitrary clip/stroke-shape shadow silhouette
//     (the WrapperView clip-path shadow) is not expressible via Outline.setRoundRect; that is the same
//     canvas/WrapperView port the arbitrary-clip deferral needs.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s AndroidNative Java support library:
// the widget test host and the gallery app host both dex every *.java here, so this class is picked up
// automatically alongside MauiImageView / MauiShapeView / MauiLayout / MauiClipOutlineProvider — no
// build-script edits.
package dev.mauicpp;

import android.graphics.Outline;
import android.view.View;
import android.view.ViewOutlineProvider;

public final class MauiShadowOutlineProvider extends ViewOutlineProvider {
    // The caster rect + corner radius, in the target View's PIXEL coordinate space (the native handler
    // resolves the view's live bounds and corner radius against the display density before constructing).
    private final int width;
    private final int height;
    private final float cornerRadius;

    public MauiShadowOutlineProvider(int width, int height, float cornerRadius) {
        this.width = width;
        this.height = height;
        this.cornerRadius = cornerRadius;
    }

    @Override
    public void getOutline(View view, Outline outline) {
        if (width <= 0 || height <= 0) {
            // No laid-out size yet — leave the outline empty; the handler re-installs from platform_arrange
            // once the view has its final size, then invalidateOutline() re-queries this.
            return;
        }
        // A rounded rect is always shadow-castable (Outline.canClip()/getAlpha aside, setRoundRect yields a
        // valid convex shadow outline). radius <= 0 gives a square-cornered rect. The outline's alpha is left
        // at the default 1 so the full shadow renders.
        outline.setRoundRect(0, 0, width, height, Math.max(0f, cornerRadius));
    }
}
