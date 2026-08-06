// dev.mauicpp.MauiDialogBridge — the JNI trampoline for the three dialog-bearing handlers
// (picker / date_picker / time_picker). One Java object per connected handler carries the peer
// (the address of that handler's dialog_trampoline, see android_dialog_ops.hpp) and implements
// every listener interface those handlers install:
//
//   View.OnClickListener               the read-only field tapped -> open the modal
//                                      (MauiDatePicker/MauiTimePicker.OnClick -> ShowPicker;
//                                       PickerHandler.Android.cs's platformView.Click += OnClick)
//   DatePickerDialog.OnDateSetListener DatePickerHandler.Android.cs's CreateDatePickerDialog callback
//   TimePickerDialog.OnTimeSetListener TimePickerHandler.Android.cs's onTimeSetCallback
//   DialogInterface.OnClickListener    the single-choice row tap (PickerHandler's SetSingleChoiceItems)
//   DialogInterface.OnDismissListener  every handler's OnDialogDismiss (IsOpen/IsFocused back to false)
//
// Why this replaced dev.mauicpp.NativeOnClickListener: JNI RegisterNatives binds per CLASS, not per
// instance, so a second registration of `nativeOnClick` on that one class REPLACED the binding
// button_handler.cpp installed, process-wide — a button tap would then land in a picker trampoline with
// a button_platform address as its peer. This class owns its own five native methods, bound once from
// android_dialog_ops.hpp, and every one of them resolves the peer through a live-peer registry before
// dereferencing it (a torn-down handler resolves to null and the callback is a no-op).
//
// This directory (src/platform/android/java/) is the port's twin of C#'s src/Core/AndroidNative Java
// support library; both the widget test host (tools/android-testhost-run.sh) and the app host
// (tools/parity/lib/build_android_apphost.sh) dex *.java from here, so no build wiring is needed.
package dev.mauicpp;

import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.content.DialogInterface;
import android.view.View;
import android.widget.DatePicker;
import android.widget.TimePicker;

public final class MauiDialogBridge
        implements View.OnClickListener,
        DatePickerDialog.OnDateSetListener,
        TimePickerDialog.OnTimeSetListener,
        DialogInterface.OnClickListener,
        DialogInterface.OnDismissListener {
    private final long peer;

    public MauiDialogBridge(long peer) {
        this.peer = peer;
    }

    @Override
    public void onClick(View view) {
        nativeOnClick(peer);
    }

    @Override
    public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
        nativeOnDateSet(peer, year, month, dayOfMonth);
    }

    @Override
    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
        nativeOnTimeSet(peer, hourOfDay, minute);
    }

    // The single-choice list callback. `which` is the row index for a list item and a negative
    // DialogInterface.BUTTON_* constant for a button; the native side ignores the negatives.
    @Override
    public void onClick(DialogInterface dialog, int which) {
        nativeOnItemSelected(peer, which);
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        nativeOnDismiss(peer);
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance is constructed. The peer is only
    // ever an OPAQUE token to Java: the native side validates it against its live-peer registry, so a
    // late callback from a dialog that outlived its handler resolves to nothing and returns.
    private static native void nativeOnClick(long peer);

    private static native void nativeOnDateSet(long peer, int year, int month, int dayOfMonth);

    private static native void nativeOnTimeSet(long peer, int hourOfDay, int minute);

    private static native void nativeOnItemSelected(long peer, int which);

    private static native void nativeOnDismiss(long peer);
}
