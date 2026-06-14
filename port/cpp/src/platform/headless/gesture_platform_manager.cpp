// gesture_platform_manager — headless platform partial (the GesturePlatformManager.Standard.cs role:
// no native gesture system). There is no native recognizer to create, so the native hooks are no-ops —
// the cross-platform core's attached_ bookkeeping IS the headless mirror (attached_count()), and the
// synthetic_* dispatch on the manager is the headless stand-in for the native gesture events: it
// drives the attached recognizers through the same controller-interface calls (send_*) the real
// bridges make, so the full pipeline (collection → manager → recognizer → event) is testable without
// natives. The Apple/iOS twins (src/platform/{apple,ios}/gesture_platform_manager.mm) attach the real
// NSGestureRecognizers / UIGestureRecognizers here instead.

#include <memory>

#include "maui/controls/gestures/gesture_platform_manager.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"

namespace maui::controls
{
    // The backend attachment table is empty on headless — there is nothing native to track (the type
    // must still be complete here for the defaulted destructor's unique_ptr deleter).
    struct gesture_native_state
    {
    };

    gesture_platform_manager::gesture_platform_manager() = default;
    gesture_platform_manager::~gesture_platform_manager() = default;

    // The headless bodies keep the shared backend seam's signatures (instance methods — the apple/ios
    // partials carry per-manager native state); native_state_ stays unallocated (reset is the no-op
    // spelling of "headless owns no native attachment").
    void gesture_platform_manager::native_attach(const std::shared_ptr<gesture_recognizer>& recognizer)
    {
        (void)recognizer; // headless: no native recognizer to create
        native_state_.reset();
    }

    void gesture_platform_manager::native_detach(const gesture_recognizer& recognizer)
    {
        (void)recognizer;
        native_state_.reset();
    }

    void gesture_platform_manager::native_detach_all()
    {
        native_state_.reset();
    }

    // --- drag&drop (W2-22): headless owns no native drag/drop registration (the recognizer still joins
    // attached_ via the cross-platform diff-sync, observable through is_attached). ---
    bool gesture_platform_manager::native_registered_drag_source(const gesture_recognizer& recognizer) const
    {
        (void)recognizer;
        return native_state_ != nullptr; // headless never builds native_state_ → always false
    }

    bool gesture_platform_manager::native_registered_drop_target(const gesture_recognizer& recognizer) const
    {
        (void)recognizer;
        return native_state_ != nullptr; // headless never builds native_state_ → always false
    }
    // --- end drag&drop (W2-22) ---
} // namespace maui::controls
