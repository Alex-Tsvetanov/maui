// dev.mauicpp.MauiImageView — an android.widget.ImageView that can clip its drawing to an arbitrary
// android.graphics.Path, the Android twin of how MAUI masks an Image's layer for VisualElement.Clip
// (iOS: WrapperView.SetClip installs a CAShapeLayer mask on the UIImageView's layer — see
// src/platform/ios/image_handler.mm image_platform::update_clip + ios_visual_ops apply_and_store_clip).
//
// WHY A CUSTOM SUBCLASS (not setClipToOutline): Android's ViewOutlineProvider + setClipToOutline only
// supports CONVEX outlines, so it cannot express the gallery's clip geometries — an arbitrary PathGeometry
// (a triangle) or a multi-ellipse GeometryGroup. Clipping the Canvas to the native-built Path in onDraw
// expresses the SAME arbitrary-shape model the iOS CAShapeLayer mask does (a non-convex mask of any shape),
// so the whole clip family (Rectangle/Ellipse/RoundRectangle/GeometryGroup/PathGeometry) renders to match
// the iOS reference. The Path's own fill type carries the fill rule: Canvas.clipPath honours whatever the
// handler set on the Path. The handler builds it with the default WINDING (non-zero) rule, matching the
// iOS WrapperView.SetClip mask (a plain CAShapeLayer whose fill rule is the CoreAnimation default
// kCAFillRuleNonZero) — so a GeometryGroup clips to the non-zero union, exactly as the iOS reference does
// (the EvenOdd hollow centre is not conveyed on either backend; see image_handler.cpp install_clip).
//
// WHY ImageView(Context) (theme-independent ctor): like MauiShapeView/MauiLayout, the bare app_process
// widget test host has no Activity theme, so a ctor resolving a defStyleAttr would throw there.
// ImageView(Context) resolves no style attr (LESSON 2 in docs/MACOS_ANDROID_RESUME.md), so MauiImageView
// constructs in both the testhost AND the real app host. Because it extends ImageView, every method the
// android image_handler already drives through the android/widget/ImageView class (setImageBitmap,
// setScaleType, setAdjustViewBounds, measure/layout) resolves unchanged — the handler swaps the plain
// ImageView for this subclass with no other change to the create/measure/aspect JNI.
//
// The clip path is a native-built android.graphics.Path the handler installs via setClipPath (null to
// remove the clip — the WrapperView.SetClip(null) analog). The handler rebuilds + reinstalls the Path on
// every clip change AND on every platform_arrange (the clip geometry is resolved against the live bounds,
// so a resize must reframe it — the iOS reapply_clip analog). onDraw saves the canvas, clips to the path,
// draws the image, and restores — the path is in the View's own (pixel) coordinate space, which is what
// the native builder produces (it scales the point-space geometry by the display density before building).
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library: the widget test host and the gallery app host both dex every *.java here
// (tools/android-testhost-run.sh + tools/parity/build_android_apphost.sh glob this dir), so MauiImageView
// is picked up automatically alongside MauiShapeView/MauiLayout/MauiDialogBridge — no script edits.
package dev.mauicpp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Path;
import android.widget.ImageView;

public final class MauiImageView extends ImageView {
    // The native-built clip Path (in this View's pixel coordinate space), or null for no clip. The handler
    // installs it via setClipPath; onDraw applies it. Volatile is unnecessary (set + read on the UI thread
    // only — the handler drives it from map_clip/platform_arrange on the same thread onDraw runs on).
    private Path clipPath;

    public MauiImageView(Context context) {
        super(context);
    }

    // Install (or clear, when null) the clip path the native handler built from the Image's Clip geometry
    // resolved against the current bounds. Schedules a redraw so onDraw re-applies it immediately.
    public void setClipPath(Path path) {
        this.clipPath = path;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        final Path clip = clipPath;
        if (clip == null) {
            super.onDraw(canvas);
            return;
        }
        // Mask the image draw to the clip path (the CAShapeLayer-mask analog). save/restore so the clip
        // does not leak into any sibling/overlay draw the parent ViewGroup performs in the same pass.
        final int checkpoint = canvas.save();
        canvas.clipPath(clip);
        super.onDraw(canvas);
        canvas.restoreToCount(checkpoint);
    }
}
