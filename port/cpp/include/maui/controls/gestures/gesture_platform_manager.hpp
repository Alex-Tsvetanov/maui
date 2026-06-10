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
