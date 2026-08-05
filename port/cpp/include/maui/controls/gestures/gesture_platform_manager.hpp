#pragma once
// maui::controls::gesture_platform_manager  <=  Microsoft.Maui.Controls.Platform.GestureManager +
// GesturePlatformManager (src/Controls/src/Core/Platform/GestureManager/GestureManager.cs + the
// per-platform GesturePlatformManager.{iOS,Standard,…}.cs partials)
//
// One per view<> instance (View._gestureManager). C#'s GestureManager watches the view's handler
// lifecycle and (re)creates a GesturePlatformManager, which subscribes to the gesture-recognizer
// collection and diff-syncs native recognizers onto the platform view (LoadRecognizers). The port
// collapses the two classes into one object with the same split of responsibilities:
//   - set_handler   = GestureManager.SetupGestureManager / DisconnectGestures (called by
//     view<>::set_handler — the port's HandlerChanged moment);
//   - load_recognizers = GesturePlatformManager.LoadRecognizers (called on every collection change —
//     the CollectionChanged subscription);
//   - the native attach/detach per recognizer is the BACKEND PARTIAL, defined per backend in
//     src/platform/<backend>/gesture_platform_manager.{cpp,mm} (headless: bookkeeping only; apple:
//     NSClick/NSPan/NSMagnification/NSTrackingArea; ios: UITap/UIPan/UIPinch/UISwipe/UIHover), exactly
//     like the C# platform partials. The destructor is backend-defined too (it releases the retained
//     native recognizers), mirroring the *_platform structs' backend-defined destructors.
//
// The synthetic_* dispatch is the headless backend's stand-in for the native gesture events: it routes
// a synthesized gesture through the SAME controller-interface calls (send_*) the platform bridges
// make — including the per-recognizer filters (tap count + button mask, pan touch points, the pinch
// IsPinching guards, the swipe threshold detection) — so the full recognizer pipeline is observable
// without natives. It never touches natives, so it works identically on every backend.
//
// Ownership (PROFILE §8): the manager keeps a strong ref to each ATTACHED recognizer (C#'s
// _gestureRecognizers dictionary keys are strong too), so a native recognizer's bridge never outlives
// its port recognizer; handler / sender / collection are non-owning back-references into the owning
// view, which outlives the manager (they are sibling members).
//
// Native arbitration (UIGestureRecognizerDelegate, iOS — GesturePlatformManager.iOS.cs's
// ShouldReceiveTouchProxy + the per-recognizer ShouldRecognizeSimultaneously blocks): on backends with
// a native delegate seam, EVERY attached native recognizer shares one arbitration delegate (created
// lazily once, reused — C#'s `_proxy`):
//   - ShouldReceiveTouch: false when the virtual view is gone / InputTransparent / disabled, or the
//     touch lands outside the platform view's hierarchy; true for a touch on the platform view itself,
//     or on a descendant when either the touch's view or the platform view already carries recognizers.
//   - ShouldRecognizeSimultaneously, per recognizer kind: tap → together iff same taps/touches on the
//     same view; swipe → true unless the OTHER recognizer's view is a scroll view; pointer (hover +
//     custom-press) → always true; pan/pinch → false (pan consults an app-level config in C#, TBD
//     here). The headless backend has no native arbitration (no delegate seam) — its synthetic
//     pipeline routes through the same per-recognizer filters directly.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/core/gesture_status.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    // The backend's opaque native-attachment table (the per-recognizer NSGestureRecognizer /
    // UIGestureRecognizer bookkeeping). DEFINED per backend in
    // src/platform/<backend>/gesture_platform_manager.{cpp,mm} — only one backend is ever compiled
    // into a build, and only that TU (where the type is complete) instantiates the unique_ptr's
    // destructor (the manager's dtor is declared here and defined there).
    struct gesture_native_state;

    // U21 (iOS arbitration): the friend seam through which the ios backend's shared arbitration
    // delegate (the ShouldReceiveTouchProxy port) reaches the manager's live handler to resolve
    // manager._handler?.VirtualView / .PlatformView FRESH on each delegate callback (C# re-reads them
    // every time; the WeakReference guard becomes a null-handler check here). Defined only in the ios
    // partial — keeps UIKit out of this header and the manager's surface unchanged on every backend.
    struct gesture_arbitration_access;

    // The synthetic pointer phases (the port-internal tag for which Send* a synthesized pointer event
    // routes to; C# has no equivalent enum — its bridges call the five Send* methods directly).
    enum class pointer_event_kind : std::uint8_t
    {
        entered,
        exited,
        moved,
        pressed,
        released,
    };

    // Which button a recognized press/click is reported as when the native event carries no button
    // identity — the C# Mac Catalyst fallback in GesturePlatformManager.iOS.cs's
    // CreatePointerRecognizer: a secondary-ONLY mask reports secondary, anything else primary. Pure
    // buttons_mask policy, shared by the apple/ios bridges (like pinch_scale_delta below).
    [[nodiscard]] constexpr buttons_mask effective_button(buttons_mask mask)
    {
        const bool secondary = contains(mask, buttons_mask::secondary);
        const bool primary = contains(mask, buttons_mask::primary);
        return (secondary && !primary) ? buttons_mask::secondary : buttons_mask::primary;
    }

    // GesturePlatformManager.iOS.cs's pinch Changed-case math: turn the platform recognizer's
    // CUMULATIVE scale reading into the RELATIVE per-update delta PinchUpdated carries —
    // 1 ± |scale - previous| * starting (starting = the attached view's render scale at pinch start).
    // Shared by the apple/ios bridges and unit-tested headless.
    [[nodiscard]] inline double pinch_scale_delta(double previous_scale, double current_scale, double starting_scale)
    {
        const double dif =
            (current_scale > previous_scale ? current_scale - previous_scale : previous_scale - current_scale) *
            starting_scale;
        if (previous_scale < current_scale)
        {
            return 1 + dif;
        }
        if (previous_scale > current_scale)
        {
            return 1 - dif;
        }
        return 1.0;
    }

    class gesture_platform_manager
    {
    public:
        // Ctor + dtor are backend-defined out-of-line (the dtor releases the retained native
        // attachments; both live where gesture_native_state is complete, so the owning unique_ptr's
        // deleter instantiates only in the backend partial).
        gesture_platform_manager();
        ~gesture_platform_manager();
        gesture_platform_manager(const gesture_platform_manager&) = delete;
        gesture_platform_manager(gesture_platform_manager&&) = delete;
        gesture_platform_manager& operator=(const gesture_platform_manager&) = delete;
        gesture_platform_manager& operator=(gesture_platform_manager&&) = delete;

        // GestureManager.SetupGestureManager / DisconnectGestures: the view's handler changed. The same
        // handler is a no-op (already set up and watching the right platform view); a different (or
        // null) handler detaches every native recognizer first, then re-attaches the whole collection
        // against the new platform view. `sender` is the owning view (the Element every send_* names).
        void set_handler(maui::core::i_view_handler* handler, element& sender,
                         gesture_recognizer_collection& recognizers);

        // GesturePlatformManager.LoadRecognizers: diff the collection against the attached set —
        // attach every collection recognizer not yet attached, detach every attached recognizer no
        // longer in the collection. A no-op until a handler is set.
        void load_recognizers();

        // Observability: how many port recognizers currently have a (native) attachment.
        [[nodiscard]] std::size_t attached_count() const
        {
            return attached_.size();
        }
        [[nodiscard]] bool is_attached(const gesture_recognizer& recognizer) const;

        // --- drag&drop (W2-22) ---------------------------------------------------------------------------
        // Native-attachment observability for the drag & drop recognizers (the attachment-only proof — a
        // synthetic dragging SESSION isn't drivable headlessly or in the spawned sim lane, so the native
        // tests assert the install/remove via these instead; documented). True iff the recognizer
        // currently has its native drag-source / drop-target registration installed. Defined per backend:
        // headless owns no native registration (always false — the recognizer still joins attached_ for
        // the diff-sync pipeline); apple/ios read the backend attachment table.
        [[nodiscard]] bool native_registered_drag_source(const gesture_recognizer& recognizer) const;
        [[nodiscard]] bool native_registered_drop_target(const gesture_recognizer& recognizer) const;
        // --- end drag&drop (W2-22) -----------------------------------------------------------------------

        // ---- synthetic dispatch (see the header comment) ----
        // A tap with `number_of_taps` taps of `button`: routed to every attached tap recognizer whose
        // number_of_taps_required matches and whose buttons mask contains `button` (the iOS bridge's
        // NumberOfTapsRequired + ButtonMaskRequired filters).
        void synthetic_tap(int number_of_taps = 1, buttons_mask button = buttons_mask::primary,
                           std::optional<maui::graphics::point> position = std::nullopt);
        // One pan phase with `touch_points` touches: routed to every attached pan recognizer whose
        // TouchPoints matches, stamped with PanGestureRecognizer.CurrentId exactly as the iOS bridge
        // stamps it (completed/canceled increment the id for the next gesture).
        void synthetic_pan(maui::core::gesture_status phase, double total_x = 0, double total_y = 0,
                           int touch_points = 1);
        // One pinch phase (scale = the relative per-update delta; origin in view-relative unit
        // coordinates). completed/canceled honor the IsPinching guard the iOS bridge applies.
        void synthetic_pinch(maui::core::gesture_status phase, double scale = 1, maui::graphics::point origin = {});
        // A finished swipe with the given totals: SendSwipe + DetectSwipe(recognizer.Direction) on
        // every attached swipe recognizer (the Windows/Android accumulate-then-detect model — the same
        // state machine the C# SwipeGestureRecognizerTests drive).
        void synthetic_swipe(double total_x, double total_y);
        // One pointer phase: hover phases (entered/exited/moved) reach every attached pointer
        // recognizer; pressed/released only those whose buttons mask contains `button` (the iOS
        // bridge's non-hover mask filter).
        void synthetic_pointer(pointer_event_kind kind, std::optional<maui::graphics::point> position = std::nullopt,
                               buttons_mask button = buttons_mask::primary);

    private:
        // U21 (iOS arbitration): grants the ios backend's arbitration delegate read access to handler_
        // (see gesture_arbitration_access above) so its UIGestureRecognizerDelegate callbacks resolve
        // the live virtual/platform view. A no-op on every other backend (the struct is only defined in
        // the ios partial).
        friend struct gesture_arbitration_access;

        // The dispatch spine EVERY synthetic_* runs through — and the reason none of them can touch
        // `attached_` or `sender_` again once user code has run.
        //
        // C#'s bridges fan out over EnumerableExtensions.GetGesturesFor, whose remark reads "The method
        // makes a defensive copy of the gestures" (`new List<IGestureRecognizer>(gestures)`,
        // src/Controls/src/Core/EnumerableExtensions.cs:48-56). The port's NATIVE bridges already copy
        // exactly that way (src/platform/android/gesture_platform_manager.cpp:331-345,
        // src/platform/windows/gesture_platform_manager.cpp:432-450); this is the same copy for the
        // cross-platform manager, which used to walk the live member vector instead.
        //
        // Why a copy of STRONG refs rather than a per-send liveness check: a Tapped/PanUpdated/… handler
        // may (a) add a recognizer — `attached_` reallocates and the walked buffer is freed; (b) remove
        // one — `attached_` erases and the recognizer is freed while its own send_* is still on the
        // stack, and pinch/drag both write their latch AFTER the raise (PinchGestureRecognizer.IsPinching,
        // DragGestureRecognizer._isDragActive); or (c) destroy the whole view — the manager is a member
        // of it, so `this` and both members go with it. Owning the recognizers for the duration of the
        // call is C#'s GC root spelled in C++: it makes every send_*'s post-raise access to its own state
        // valid without any send_* knowing about it, and gives the walk memory user code cannot reach.
        //
        // `sender` is read ONCE, before any user code: an element is not shared-owned, so the port cannot
        // root it — no send_* may dereference `sender` after raising (see gesture_recognizer.hpp).
        //
        // Snapshot semantics fall out, and match .NET's: a recognizer added mid-dispatch is not observed
        // until the next gesture; one removed mid-dispatch still receives the in-flight one.
        template <class Recognizer, class Body> void dispatch(Body&& body)
        {
            if (sender_ == nullptr)
            {
                return;
            }
            element& sender = *sender_;
            const std::vector<std::shared_ptr<gesture_recognizer>> recognizers = attached_;
            for (const std::shared_ptr<gesture_recognizer>& recognizer : recognizers)
            {
                if (auto* const typed = dynamic_cast<Recognizer*>(recognizer.get()); typed != nullptr)
                {
                    body(*typed, sender);
                }
            }
        }

        // The backend partial (src/platform/<backend>/gesture_platform_manager.{cpp,mm}): attach /
        // detach the native recognizer(s) for one port recognizer on handler_->native_view().
        // native_state_ is the backend's opaque attachment table (headless leaves it null).
        void native_attach(const std::shared_ptr<gesture_recognizer>& recognizer);
        void native_detach(const gesture_recognizer& recognizer);
        void native_detach_all();

        maui::core::i_view_handler* handler_ = nullptr;             // non-owning (the view owns the handler)
        element* sender_ = nullptr;                                 // non-owning back-ref to the owning view
        gesture_recognizer_collection* recognizers_ = nullptr;      // non-owning (sibling member of the view)
        std::vector<std::shared_ptr<gesture_recognizer>> attached_; // strong, like C#'s dictionary keys
        // The backend-owned attachment table (complete only in the backend partial — see the forward
        // declaration above; the backend-defined dtor is where the unique_ptr's deleter instantiates).
        std::unique_ptr<gesture_native_state> native_state_;
    };
} // namespace maui::controls
