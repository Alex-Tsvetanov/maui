#pragma once
// maui::controls::gesture_recognizer  <=  Microsoft.Maui.Controls.GestureRecognizer
// (an Element implementing IGestureRecognizer : INotifyPropertyChanged)
//
// The base class for all gesture recognizers. Deriving the port's element base gives a recognizer the
// same surface the C# original gets from Element: bindable properties (whose changes raise
// bindable_object::property_changed — the INotifyPropertyChanged role of IGestureRecognizer) and a
// logical parent + inherited BindingContext once it is added to a view's gesture_recognizers()
// collection (View's CollectionChanged handler sets item.Parent = this; view<> mirrors that by
// attaching the recognizer as a logical child).
//
// The headers live under include/maui/controls/gestures/ for organization only — the namespace stays
// maui::controls, matching the C# namespace (Microsoft.Maui.Controls has no Gestures sub-namespace).
//
// ---- THE send_* LIFETIME CONTRACT (read before adding one) --------------------------------------
// Every send_* raises user code, and that user code may tear down the very tree it was called from —
// clear the view's gesture_recognizers() (freeing the recognizer), or destroy the sender view itself.
// In C# none of this is visible: `this` and every parameter are GC roots for the whole method body, so
// SendPinchStarted can write its IsPinching latch after the Invoke and SendDrop can read the DragSource
// element after it. The port has no GC, so the same code reads freed memory. Two rules replace it, and
// between them the whole class is closed WITHOUT any send_* carrying a lifetime guard of its own:
//
//   1. THE CALLER OWNS THE RECOGNIZER for the whole send_* call — a defensive copy of strong refs taken
//      before the first send, which is exactly what C#'s own fan-outs do
//      (EnumerableExtensions.GetGesturesFor: "The method makes a defensive copy of the gestures").
//      gesture_platform_manager::dispatch<> (the cross-platform synthetic dispatch) and the windows
//      bridge honor it, and every drag/drop fan-out does. It is NOT universal, and it is NOT a licence
//      for post-raise self-access: the android PINCH path breaks it today — pinch_gesture() returns
//      `entry.lock().get()` (src/platform/android/gesture_platform_manager.cpp:377-387), so the strong
//      ref dies at the end of that expression and the send at :1203 runs through a raw pointer. Rule 3
//      is what actually closes the callee side.
//
//   2. NO send_* MAY DEREFERENCE A BORROWED ELEMENT AFTER RAISING. `sender` (and drop's `parent`) are
//      raw element references; an element is not shared-owned, so nothing in this layer can root one.
//      Any element read a send_* needs must be taken BEFORE the raise — see the LIFETIME notes in
//      send_drag_starting / send_drop, whose precomputed reads are the documented deviation from C#'s
//      read-after-Invoke. The one irreducible exception is drop's injection into `parent`, which by
//      definition must write into a live target after the handler has run; its liveness is the caller's
//      contract (drop_gesture_recognizer.hpp).
//
//   3. NO send_* MAY WRITE ITS OWN STATE AFTER RAISING. Latch first, raise last: `this` is freed the
//      moment a handler destroys the view that owns the recognizer, and rule 1 is not guaranteed by
//      every caller (see above). Where a send_* has more than one raise site, route them through one
//      private latch-then-raise helper so the ordering cannot be re-broken by the next send_* added —
//      pinch_gesture_recognizer::latch_then_raise is the pattern (it also carries the fidelity argument
//      for moving the write, which C# does after the Invoke). Checked 2026-08-05: pan, pointer, tap and
//      swipe raise LAST already and touch nothing afterwards; drag's send_drop_completed latches before
//      its raise. The one irreducible exception is send_drag_starting's `is_drag_active_ = true`
//      (drag_drop_recognizers.cpp:83), whose value C# gates on the handler-filled args.Cancel/Handled,
//      so it cannot be hoisted — it rests on rule 1 alone, and both of its callers do snapshot strong
//      refs (android :697, windows :699).

#include "maui/controls/element.hpp"

namespace maui::controls
{
    class gesture_recognizer : public element
    {
    public:
        gesture_recognizer() = default;
    };
} // namespace maui::controls
