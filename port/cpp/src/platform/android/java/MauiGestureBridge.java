// dev.mauicpp.MauiGestureBridge — the JNI gesture trampoline for the Android backend, the port's
// twin of the Java-side objects C#'s GesturePlatformManager.Android.cs owns:
//
//   Microsoft.Maui.Controls.Platform.TapAndPanGestureDetector  (: android.view.GestureDetector)
//   Microsoft.Maui.Controls.Platform.InnerGestureListener      (: GestureDetector.IOnGestureListener,
//                                                                 GestureDetector.IOnDoubleTapListener)
//   Microsoft.Maui.Controls.Platform.InnerScaleListener        (: ScaleGestureDetector.SimpleOnScaleGestureListener)
//   Microsoft.Maui.Controls.Platform.PointerGestureHandler     (: View.IOnHoverListener)
//   Microsoft.Maui.Controls.Platform.DragAndDropGestureHandler (: View.IOnDragListener)
//   the manager's own View.Touch / View.KeyPress subscriptions (SetupGestures :241-249)
//
// ONE object per view, exactly like C#: android.view.GestureDetector and ScaleGestureDetector are
// per-VIEW detectors, not per-recognizer, so the C++ side's per-recognizer attach/detach collapses
// into one idempotent whole-collection subscription sync (see gesture_platform_manager.cpp).
//
// SPLIT OF RESPONSIBILITY: this class is DUMB PLUMBING. It owns the two platform detectors, routes
// their callbacks across JNI as primitives (action, coordinates, button state, pointer count), and
// owns nothing else. Every decision C# makes in InnerGestureListener / the five *GestureHandler
// classes — the double-tap state machine, the button-mask filters, the px->dp translation, the pan
// id stamping, the swipe accumulate-then-detect, the drag/drop payload — lives in C++, where the
// recognizer collection lives. Nothing here reads a gesture recognizer.
//
// Bound from C++ with RegisterNatives (reflection-free, no Java_* export), exactly like
// NativeOnClickListener. `peer` is the backend's gesture_native_state; the C++ side zeroes it
// through detach() BEFORE it drops this object, so a MotionEvent still queued in the platform's
// detectors can never reach a freed peer (C#'s "resurrect the eagerly-disposed listener" hazard —
// InnerGestureListener.cs:69-74 — spelled as an explicit invalidation instead).
package dev.mauicpp;

import android.content.ClipData;
import android.content.ClipDescription;
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.view.DragEvent;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

public final class MauiGestureBridge
        implements View.OnTouchListener,
                   View.OnHoverListener,
                   View.OnDragListener,
                   View.OnKeyListener,
                   GestureDetector.OnGestureListener,
                   GestureDetector.OnDoubleTapListener,
                   ScaleGestureDetector.OnScaleGestureListener {

    // DragAndDropGestureHandler.CustomLocalStateData (DragAndDropGestureHandler.cs:379-385), reduced to
    // the one field that has to cross the platform: the DRAG SOURCE's peer. C# also parks the
    // DataPackage / AcceptedOperation / SourceElement in here; in the port those stay in the source
    // view's C++ state (an in-process drag never leaves the process), reached back through this peer.
    public static final class LocalState {
        final long sourcePeer;

        LocalState(long sourcePeer) {
            this.sourcePeer = sourcePeer;
        }
    }

    private long peer;
    private final GestureDetector tapAndPanAndSwipeDetector;
    private final ScaleGestureDetector scaleDetector;
    private boolean hasPinchGestures;

    // The View supplies its own Context, exactly as C# reads Control.Context (:129-132, :155-158).
    public MauiGestureBridge(View view, long peer) {
        final Context context = view.getContext();
        this.peer = peer;
        // GesturePlatformManager.InitializeTapAndPanAndSwipeDetector :127-151 / InitializeScaleDetector
        // :153-166. Long press starts OFF — TapAndPanGestureDetector.UpdateLongPressSettings :26-39
        // turns it on only while a DragGestureRecognizer is attached (a long press pre-empts a pan).
        this.tapAndPanAndSwipeDetector = new GestureDetector(context, this);
        this.tapAndPanAndSwipeDetector.setOnDoubleTapListener(this);
        this.tapAndPanAndSwipeDetector.setIsLongpressEnabled(false);
        this.scaleDetector = new ScaleGestureDetector(context, this);
        this.scaleDetector.setQuickScaleEnabled(true); // ScaleGestureDetectorCompat.SetQuickScaleEnabled :162
    }

    // TapAndPanGestureDetector.UpdateLongPressSettings :26-39 — re-run on every collection change.
    public void setLongPressEnabled(boolean enabled) {
        tapAndPanAndSwipeDetector.setIsLongpressEnabled(enabled);
    }

    // GesturePlatformManager.ViewHasPinchGestures :168-181, cached by the C++ subscription sync so the
    // routing below stays a field read (C# re-walks the collection per touch event).
    public void setHasPinchGestures(boolean value) {
        hasPinchGestures = value;
    }

    // Invalidate the C++ back-reference. Called by native_detach_all BEFORE the peer's storage dies.
    public void detach() {
        peer = 0;
    }

    // ---- View.Touch (SetupGestures :241) -> GesturePlatformManager.OnTouchEvent :64-91 ----------------

    @Override
    public boolean onTouch(View v, MotionEvent e) {
        if (peer == 0 || e == null) {
            return false;
        }
        // The IsEnabled / InputTransparent bail (:71-74) is the platform View's own concern here: the
        // port pushes IsEnabled onto View.setEnabled and InputTransparent onto the touch-listener
        // install, so a disabled/transparent view never reaches this callback.
        boolean eventConsumed = false;
        if (hasPinchGestures) {
            eventConsumed = scaleDetector.onTouchEvent(e);
        }
        if (!hasPinchGestures || !scaleDetector.isInProgress()) {
            eventConsumed = onTapAndPanTouchEvent(e) || eventConsumed;
        }
        return eventConsumed;
    }

    // TapAndPanGestureDetector.OnTouchEvent :41-57.
    private boolean onTapAndPanTouchEvent(MotionEvent e) {
        boolean baseHandled = tapAndPanAndSwipeDetector.onTouchEvent(e);

        // getActionMasked() rather than getAction(): identical for the four actions C# tests, and it
        // does not carry the pointer-index bits a multi-touch stream would otherwise smuggle in.
        final int action = e.getActionMasked();
        boolean pointerHandled = false;
        if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_DOWN
                || action == MotionEvent.ACTION_MOVE || action == MotionEvent.ACTION_CANCEL) {
            pointerHandled = nativeOnPointerTouch(peer, action, e.getX(), e.getY(), e.getButtonState(),
                                                  actionButtonOf(e));
        }

        if (action == MotionEvent.ACTION_UP) {
            nativeEndScrolling(peer); // InnerGestureListener.EndScrolling :258-267
        }
        return baseHandled || pointerHandled;
    }

    // PointerGestureHandler.GetPressedButton :128-138 — ActionButton is API 23+ and only meaningful for
    // the explicit ButtonPress/ButtonRelease actions; 0 means "the C++ side must infer from ButtonState".
    private static int actionButtonOf(MotionEvent e) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                && (e.getActionMasked() == MotionEvent.ACTION_BUTTON_PRESS
                    || e.getActionMasked() == MotionEvent.ACTION_BUTTON_RELEASE)) {
            return e.getActionButton();
        }
        return 0;
    }

    // ---- View.OnHoverListener (PointerGestureHandler.OnHover :25-55) ---------------------------------

    @Override
    public boolean onHover(View v, MotionEvent e) {
        if (peer != 0 && e != null) {
            nativeOnHover(peer, e.getActionMasked(), e.getX(), e.getY(), e.getButtonState());
        }
        return false; // C#: PointerGestureHandler.OnHover always returns false
    }

    // ---- View.KeyPress (SetupGestures :247 -> OnKeyPress :258-287) ------------------------------------

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent e) {
        if (peer == 0 || e == null || e.getAction() != KeyEvent.ACTION_UP) {
            return false;
        }
        if (!isConfirmKey(keyCode) || !e.hasNoModifiers()) {
            return false;
        }
        if (!v.isEnabled()) {
            return true; // OnKeyPress :276-280 — swallow the key on a disabled view
        }
        if (!e.isCanceled()) {
            // SendTapped(View, (v) => Point.Zero): the accessibility tap carries no meaningful position.
            nativeOnAccessibilityTap(peer);
        }
        return false;
    }

    // ViewExtensions.IsConfirmKey (src/Core/src/Platform/Android/ViewExtensions.cs:895-907).
    private static boolean isConfirmKey(int keyCode) {
        return keyCode == KeyEvent.KEYCODE_DPAD_CENTER
                || keyCode == KeyEvent.KEYCODE_ENTER
                || keyCode == KeyEvent.KEYCODE_SPACE
                || keyCode == KeyEvent.KEYCODE_NUMPAD_ENTER;
    }

    // ---- View.OnDragListener (DragAndDropGestureHandler.OnDrag :111-160) -----------------------------

    @Override
    public boolean onDrag(View v, DragEvent e) {
        if (peer == 0 || e == null) {
            return false;
        }
        final Object local = e.getLocalState();
        final long sourcePeer = (local instanceof LocalState) ? ((LocalState) local).sourcePeer : 0L;

        // HandleDrop :209-225 — only a drag from OUTSIDE the process (no local state) needs its text
        // coerced off the ClipData; an in-process drag carries the DataPackage in the source's C++ state.
        String clipText = null;
        if (e.getAction() == DragEvent.ACTION_DROP && local == null) {
            final ClipData data = e.getClipData();
            if (data != null && data.getItemCount() > 0) {
                final CharSequence coerced = data.getItemAt(0).coerceToText(v.getContext());
                clipText = coerced == null ? null : coerced.toString();
            } else {
                final ClipDescription description = e.getClipDescription();
                final CharSequence label = description == null ? null : description.getLabel();
                clipText = label == null ? null : label.toString();
            }
        }

        nativeOnDrag(peer, e.getAction(), e.getX(), e.getY(), sourcePeer, clipText);
        return true; // C#: OnDrag always returns true
    }

    // DragAndDropGestureHandler.OnLongPress :339-354, called FROM C++ once SendDragStarting has run and
    // the args were not canceled. TEXT payload only — the image / custom ClipData paths carry a
    // // TODO: verify against src/Controls/src/Core/Platform/Android/DragAndDropGestureHandler.cs:286-334
    // marker on the C++ side (ImageSource marshalling has no port equivalent yet).
    public static void startDrag(View v, String text, long sourcePeer) {
        final String payload = text == null ? "" : text;
        // :298-307 — an absolute URI becomes a text/uri-list item, anything else text/plain.
        final Uri parsed = Uri.parse(payload);
        final ClipData data = parsed.isAbsolute() ? ClipData.newRawUri("", parsed)
                                                  : ClipData.newPlainText("", payload);
        // :342-352 — DRAG_FLAG_GLOBAL | DRAG_FLAG_GLOBAL_URI_READ on API 24+ (the port targets 34).
        v.startDragAndDrop(data, new View.DragShadowBuilder(v), new LocalState(sourcePeer),
                           View.DRAG_FLAG_GLOBAL | View.DRAG_FLAG_GLOBAL_URI_READ);
    }

    // ---- GestureDetector.IOnGestureListener (InnerGestureListener :111-175) --------------------------

    @Override
    public boolean onDown(MotionEvent e) {
        return peer != 0 && nativeOnDown(peer, e.getRawX(), e.getRawY());
    }

    @Override
    public void onShowPress(MotionEvent e) {
        // InnerGestureListener.OnShowPress :149-151 — deliberately empty.
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {
        return peer != 0 && nativeOnSingleTapUp(peer, e.getX(), e.getY(), e.getButtonState());
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
        if (peer == 0 || e1 == null || e2 == null) {
            return false; // InnerGestureListener.OnScroll :141-142
        }
        return nativeOnScroll(peer, e1.getRawX(), e1.getRawY(), e2.getRawX(), e2.getRawY(),
                              e2.getPointerCount());
    }

    @Override
    public void onLongPress(MotionEvent e) {
        if (peer != 0) {
            nativeOnLongPress(peer, e.getRawX(), e.getRawY());
        }
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        if (peer != 0) {
            nativeEndScrolling(peer); // InnerGestureListener.OnFling :127-131
        }
        return false;
    }

    // ---- GestureDetector.IOnDoubleTapListener (InnerGestureListener :76-109, :177-204) ---------------

    @Override
    public boolean onDoubleTap(MotionEvent e) {
        return peer != 0 && nativeOnDoubleTap(peer, e.getX(), e.getY(), e.getButtonState());
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e) {
        return peer != 0
                && nativeOnDoubleTapEvent(peer, e.getActionMasked(), e.getX(), e.getY(), e.getButtonState());
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        return peer != 0 && nativeOnSingleTapConfirmed(peer, e.getX(), e.getY(), e.getButtonState());
    }

    // ---- ScaleGestureDetector.OnScaleGestureListener (InnerScaleListener :36-55) ---------------------
    // The focus is handed over in PIXELS; the C++ side applies context.FromPixels (InnerScaleListener
    // :44, :49) with the view's own display density. The span pair carries the :41-42 deadband inputs.

    @Override
    public boolean onScale(ScaleGestureDetector d) {
        return peer != 0
                && nativeOnScale(peer, d.getScaleFactor(), d.getCurrentSpan(), d.getPreviousSpan(),
                                 d.getFocusX(), d.getFocusY());
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector d) {
        return peer != 0 && nativeOnScaleBegin(peer, d.getFocusX(), d.getFocusY());
    }

    @Override
    public void onScaleEnd(ScaleGestureDetector d) {
        if (peer != 0) {
            nativeOnScaleEnd(peer);
        }
    }

    // ---- the native halves (bound via RegisterNatives before any instance is constructed) ------------

    private static native boolean nativeOnDown(long peer, float rawX, float rawY);

    private static native boolean nativeOnScroll(long peer, float startRawX, float startRawY,
                                                 float rawX, float rawY, int pointerCount);

    private static native void nativeEndScrolling(long peer);

    private static native void nativeOnLongPress(long peer, float rawX, float rawY);

    private static native boolean nativeOnSingleTapUp(long peer, float x, float y, int buttonState);

    private static native boolean nativeOnSingleTapConfirmed(long peer, float x, float y, int buttonState);

    private static native boolean nativeOnDoubleTap(long peer, float x, float y, int buttonState);

    private static native boolean nativeOnDoubleTapEvent(long peer, int action, float x, float y,
                                                         int buttonState);

    private static native void nativeOnAccessibilityTap(long peer);

    private static native boolean nativeOnPointerTouch(long peer, int action, float x, float y,
                                                       int buttonState, int actionButton);

    private static native void nativeOnHover(long peer, int action, float x, float y, int buttonState);

    private static native boolean nativeOnScale(long peer, float scaleFactor, float currentSpan,
                                                float previousSpan, float focusX, float focusY);

    private static native boolean nativeOnScaleBegin(long peer, float focusX, float focusY);

    private static native void nativeOnScaleEnd(long peer);

    private static native void nativeOnDrag(long peer, int action, float x, float y, long sourcePeer,
                                            String clipText);
}
