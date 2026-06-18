// maui::controls — out-of-line definitions for the gesture recognizers: the shared bindable-property
// descriptors (one per property, like the C# static BindableProperty fields), pan's shared CurrentId
// minter, and the SwipeGestureExtensions rotation transform. Ported from TapGestureRecognizer.cs /
// PanGestureRecognizer.cs / SwipeGestureRecognizer.cs / PointerGestureRecognizer.cs /
// Internals/SwipeGestureExtensions.cs (defaults preserved).

#include <cmath>
#include <cstdint>
#include <memory> // (U-CMD) std::shared_ptr in the *Command property descriptors

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
#include "maui/controls/gestures/drop_gesture_recognizer.hpp" // --- drag&drop (W2-22) ---
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/controls/i_command.hpp" // (U-CMD) the *Command property value type
#include "maui/core/bindable_property.hpp"
#include "maui/core/swipe_direction.hpp"

namespace maui::controls
{
    // ---- tap_gesture_recognizer ----
    const maui::core::bindable_property<std::shared_ptr<i_command>>& tap_gesture_recognizer::command_property()
    {
        // TapGestureRecognizer.CommandProperty (default null — an unset shared_ptr).
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"command", nullptr};
        return descriptor;
    }

    const maui::core::bindable_property<int>& tap_gesture_recognizer::number_of_taps_required_property()
    {
        // TapGestureRecognizer.NumberOfTapsRequiredProperty (default 1).
        static const maui::core::bindable_property<int> descriptor{"number_of_taps_required", 1};
        return descriptor;
    }

    const maui::core::bindable_property<buttons_mask>& tap_gesture_recognizer::buttons_property()
    {
        // TapGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask> descriptor{"buttons", buttons_mask::primary};
        return descriptor;
    }

    // ---- pan_gesture_recognizer ----
    auto_id& pan_gesture_recognizer::current_id()
    {
        // PanGestureRecognizer.CurrentId (one shared minter, like the C# static property).
        static auto_id shared;
        return shared;
    }

    const maui::core::bindable_property<int>& pan_gesture_recognizer::touch_points_property()
    {
        // PanGestureRecognizer.TouchPointsProperty (default 1).
        static const maui::core::bindable_property<int> descriptor{"touch_points", 1};
        return descriptor;
    }

    // ---- swipe_gesture_recognizer ----
    const maui::core::bindable_property<maui::core::swipe_direction>& swipe_gesture_recognizer::direction_property()
    {
        // SwipeGestureRecognizer.DirectionProperty (default: default(SwipeDirection) — no direction).
        static const maui::core::bindable_property<maui::core::swipe_direction> descriptor{
            "direction", maui::core::swipe_direction::none};
        return descriptor;
    }

    const maui::core::bindable_property<std::uint32_t>& swipe_gesture_recognizer::threshold_property()
    {
        // SwipeGestureRecognizer.ThresholdProperty (default DefaultSwipeThreshold = 100 px).
        static const maui::core::bindable_property<std::uint32_t> descriptor{"threshold", 100U};
        return descriptor;
    }

    // ---- pointer_gesture_recognizer ----
    const maui::core::bindable_property<buttons_mask>& pointer_gesture_recognizer::buttons_property()
    {
        // PointerGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask> descriptor{"buttons", buttons_mask::primary};
        return descriptor;
    }

    // The five Pointer*CommandProperty descriptors (PointerGestureRecognizer.Pointer*CommandProperty,
    // default null — an unset shared_ptr).
    const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_gesture_recognizer::
        pointer_entered_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"pointer_entered_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_gesture_recognizer::
        pointer_exited_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"pointer_exited_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_gesture_recognizer::
        pointer_moved_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"pointer_moved_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_gesture_recognizer::
        pointer_pressed_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"pointer_pressed_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& pointer_gesture_recognizer::
        pointer_released_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"pointer_released_command",
                                                                                          nullptr};
        return descriptor;
    }

    // --- drag&drop (W2-22) ---
    // ---- drag_gesture_recognizer ----
    const maui::core::bindable_property<bool>& drag_gesture_recognizer::can_drag_property()
    {
        // DragGestureRecognizer.CanDragProperty (default true).
        static const maui::core::bindable_property<bool> descriptor{"can_drag", true};
        return descriptor;
    }

    // DragGestureRecognizer.DragStartingCommandProperty / DropCompletedCommandProperty (default null).
    const maui::core::bindable_property<std::shared_ptr<i_command>>& drag_gesture_recognizer::
        drag_starting_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"drag_starting_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& drag_gesture_recognizer::
        drop_completed_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"drop_completed_command",
                                                                                          nullptr};
        return descriptor;
    }

    // ---- drop_gesture_recognizer ----
    const maui::core::bindable_property<bool>& drop_gesture_recognizer::allow_drop_property()
    {
        // DropGestureRecognizer.AllowDropProperty (default true).
        static const maui::core::bindable_property<bool> descriptor{"allow_drop", true};
        return descriptor;
    }

    // DropGestureRecognizer.DragOverCommandProperty / DragLeaveCommandProperty / DropCommandProperty
    // (default null).
    const maui::core::bindable_property<std::shared_ptr<i_command>>& drop_gesture_recognizer::
        drag_over_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"drag_over_command", nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& drop_gesture_recognizer::
        drag_leave_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"drag_leave_command",
                                                                                          nullptr};
        return descriptor;
    }
    const maui::core::bindable_property<std::shared_ptr<i_command>>& drop_gesture_recognizer::drop_command_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_command>> descriptor{"drop_command", nullptr};
        return descriptor;
    }
    // --- end drag&drop (W2-22) ---

    // ---- Internals.SwipeGestureExtensions.TransformSwipeDirectionForRotation ----
    maui::core::swipe_direction transform_swipe_direction_for_rotation(maui::core::swipe_direction direction,
                                                                       double rotation)
    {
        using maui::core::swipe_direction;

        if (std::isnan(rotation) || std::isinf(rotation))
        {
            return direction;
        }

        // Normalize to [0, 360), then round to the nearest 90° increment; only transform when the
        // rotation is within 45° of a cardinal angle (arbitrary rotations leave the direction as-is).
        const double normalized_rotation = normalize_rotation(rotation);
        const double rotation_rounded = std::round(normalized_rotation / 90.0) * 90.0;
        if (std::abs(normalized_rotation - rotation_rounded) > 45.0)
        {
            return direction;
        }

        // Rotation steps as multiples of 90° (0=0°, 1=90°, 2=180°, 3=270°).
        const int rotation_steps = static_cast<int>(rotation_rounded / 90.0) % 4;
        switch (rotation_steps)
        {
            case 1: // 90° clockwise
                switch (direction)
                {
                    case swipe_direction::up:
                        return swipe_direction::right;
                    case swipe_direction::right:
                        return swipe_direction::down;
                    case swipe_direction::down:
                        return swipe_direction::left;
                    case swipe_direction::left:
                        return swipe_direction::up;
                    default:
                        return direction;
                }
            case 2: // 180°
                switch (direction)
                {
                    case swipe_direction::up:
                        return swipe_direction::down;
                    case swipe_direction::right:
                        return swipe_direction::left;
                    case swipe_direction::down:
                        return swipe_direction::up;
                    case swipe_direction::left:
                        return swipe_direction::right;
                    default:
                        return direction;
                }
            case 3: // 270° clockwise (90° counter-clockwise)
                switch (direction)
                {
                    case swipe_direction::up:
                        return swipe_direction::left;
                    case swipe_direction::right:
                        return swipe_direction::up;
                    case swipe_direction::down:
                        return swipe_direction::right;
                    case swipe_direction::left:
                        return swipe_direction::down;
                    default:
                        return direction;
                }
            default: // no rotation
                return direction;
        }
    }
} // namespace maui::controls
