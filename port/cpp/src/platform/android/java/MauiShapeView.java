// dev.mauicpp.MauiShapeView — the custom android.view.View the Android shape backend draws into, the
// C++ analog of Microsoft.Maui.Platform.MauiShapeView : PlatformGraphicsView (and its base
// PlatformGraphicsView). MAUI's Android shape view is a View whose OnDraw(Canvas) wraps the
// android.graphics.Canvas in a Microsoft.Maui.Graphics.Platform.PlatformCanvas and calls the
// ShapeDrawable's Draw(ICanvas). This class is the same: onDraw hands the Canvas across JNI to
// nativeDraw, where the native side (box_view_handler.cpp's android partial) builds an android_canvas
// over it and replays the shared shape_drawable (shape_view_platform::replay).
//
// WHY A PLAIN View(Context) ctor (theme-independent): like MauiLayout, the bare app_process widget test
// host has no Activity theme, so a ctor that resolves a defStyleAttr would throw there. View(Context)
// resolves no style attr (LESSON 2 in docs/MACOS_ANDROID_RESUME.md), so MauiShapeView constructs in both
// the testhost AND the real app host. The handler frames it absolutely (View.layout) and seeds nativePtr;
// invalidate() (a plain View method, used by the handler on every shape/fill change) schedules onDraw.
//
// nativePtr is the address of the handler's shape_view_platform; the handler sets it after creating the
// view and clears it (setNativePtr(0)) before the struct can die, so onDraw never dereferences a dangling
// pointer (a 0 ptr makes nativeDraw a no-op). nativeDraw is bound from C++ via RegisterNatives (the
// reflection-free NativeOnClickListener recipe) — no Java_* export symbol is needed.
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library: the widget test host and the gallery app host both dex every *.java here
// (tools/android-testhost-run.sh + tools/parity/build_android_apphost.sh glob this dir), so MauiShapeView
// is picked up automatically alongside MauiLayout/NativeOnClickListener — no build-script edits needed.
package dev.mauicpp;

import android.content.Context;
import android.graphics.Canvas;
import android.view.View;

public final class MauiShapeView extends View {
    private long nativePtr;

    public MauiShapeView(Context context) {
        super(context);
        // The drawing host is transparent: the shape paints its own fill/stroke; the View must not paint a
        // background of its own (C# PlatformGraphicsView sets the view non-opaque). The default View
        // background is already null/transparent, so nothing to clear — kept explicit for intent.
    }

    // The handler installs the peer (the shape_view_platform address) after construction and clears it
    // (0) on disconnect, before the native struct is destroyed.
    public void setNativePtr(long ptr) {
        this.nativePtr = ptr;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (nativePtr != 0L) {
            // The native side builds an android_canvas over `canvas` and replays the shape drawable. The
            // Canvas is valid for this call only (the View owns it) — the native side never retains it.
            nativeDraw(nativePtr, canvas, getWidth(), getHeight());
        }
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance draws (box_view_handler.cpp's android
    // partial). The peer is the handler's shape_view_platform, valid while the view is connected.
    private static native void nativeDraw(long peer, Canvas canvas, int width, int height);
}
