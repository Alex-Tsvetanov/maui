// dev.mauicpp.MauiCheckedItemInflaterFactory — a one-tag LayoutInflater.Factory2 for
// MaterialAlertDialogBuilder's single-choice item rows.
//
// WHY THIS EXISTS: MaterialAlertDialogBuilder.setSingleChoiceItems inflates its rows from
// mtrl_alert_select_dialog_singlechoice.xml, an unqualified `<CheckedTextView app:drawableStartCompat=
// "?android:attr/listChoiceIndicatorSingle" .../>` tag. `app:drawableStartCompat` is an AppCompat-only
// attribute, honoured only when the LayoutInflater doing the inflating has AppCompat's Factory2
// installed — that factory is what swaps the plain "CheckedTextView" tag for
// androidx.appcompat.widget.AppCompatCheckedTextView, which is the class that actually knows how to
// read app:drawableStartCompat and paint the radio-ring indicator. Without the swap the row inflates as
// framework android.widget.CheckedTextView, which has never heard of that attribute and silently drops
// it — confirmed live via `adb shell uiautomator dump` (class="android.widget.CheckedTextView", not
// AppCompatCheckedTextView) and a JNI probe of the exact LayoutInflater instance (see below).
//
// WHY THE FACTORY ISN'T ALREADY THERE: AppCompatDialog's own delegate DOES install a full
// AppCompatDelegateImpl factory, correctly, during onCreate() — but only on the DIALOG's own
// ContextThemeWrapper (`dialog.getContext()`). The row adapter inflates through a DIFFERENT
// ContextThemeWrapper: the one `new MaterialAlertDialogBuilder(context)` mints for itself
// (`builder.getContext()`), which AlertParams/CheckedItemAdapter capture via `LayoutInflater.from(...)`
// at construction time. In a normal AppCompatActivity-hosted app this gap is invisible, because the
// Activity's OWN base LayoutInflater already carries AppCompat's factory (installed by the Activity's
// own onCreate), and every ContextThemeWrapper's lazily-cloned inflater
// (LayoutInflater#cloneInContext) inherits whatever factory its SOURCE inflater had at clone time. This
// backend's host (MauiHostActivity) is a plain android.app.Activity — no AppCompat AAR dependency
// there, so `LayoutInflater.from(activityContext)` never gets a factory from anywhere, and neither does
// any wrapper cloned from it. Verified empirically (android_dialog_ops.hpp's now-removed diagnostic
// probes): post-show, `LayoutInflater.from(builder.getContext())` and `LayoutInflater.from(
// dialog.getContext())` are two DIFFERENT instances; only the latter carries AppCompatDelegateImpl.
//
// THE FIX: android_dialog_ops.hpp's show_items_dialog installs ONE instance of this class on
// `LayoutInflater.from(builder.getContext())` — right after the builder is constructed, before
// create() (which is when AlertParams captures/uses that inflater) — guarded by getFactory2() == null
// so a second picker/ios_picker open on the SAME Activity (a NEW builder, but MaterialAlertDialogBuilder
// always mints a fresh ContextThemeWrapper per instance, so this runs once per open, cheaply) never
// double-installs (LayoutInflater#setFactory2 throws if a factory is already set).
//
// SCOPE: deliberately narrow — only the "CheckedTextView" tag, only on the builder's own inflater.
// date_picker/time_picker are framework android.app.DatePickerDialog/TimePickerDialog, entirely
// unaffected. Everything else Material's row layout needs (padding, text appearance, colours) is a
// plain android: attribute any TextView already honours without this factory.
package dev.mauicpp;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;

import androidx.appcompat.widget.AppCompatCheckedTextView;

public final class MauiCheckedItemInflaterFactory implements LayoutInflater.Factory2 {
    @Override
    public View onCreateView(View parent, String name, Context context, AttributeSet attrs) {
        return onCreateView(name, context, attrs);
    }

    @Override
    public View onCreateView(String name, Context context, AttributeSet attrs) {
        if ("CheckedTextView".equals(name)) {
            return new AppCompatCheckedTextView(context, attrs);
        }
        return null; // let the inflater fall through to its own default view creation
    }
}
