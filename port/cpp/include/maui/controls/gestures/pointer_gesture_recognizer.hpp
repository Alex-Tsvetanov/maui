#pragma once
// maui::controls::pointer_gesture_recognizer  <=  Microsoft.Maui.Controls.PointerGestureRecognizer
// maui::controls::pointer_event_args          <=  Microsoft.Maui.Controls.PointerEventArgs
//
// Provides pointer gesture recognition and events: entered / exited / moved over the attached view
// (hover), plus pressed / released. Ported from PointerGestureRecognizer.cs + PointerEventArgs.cs.
// Deviations (documented, port-wide):
//   - The five Pointer*Command/Pointer*CommandParameter pairs are not ported (no ICommand port yet —
//     STATUS.md); each Send* collapses to raising its event.
//   - PointerEventArgs.GetPosition(relativeTo) / PlatformArgs are narrowed to a stored view-relative
//     position (nullopt when unavailable), like tapped_event_args.

#include <optional>

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls
{
    // The payload of the pointer events (PointerEventArgs). Button is the mouse button that triggered
    // the event; for pure hover events it stays primary (the C# default).
    struct pointer_event_args
    {
        buttons_mask button = buttons_mask::primary;
        std::optional<maui::graphics::point> position;
    };

    class pointer_gesture_recognizer final : public gesture_recognizer
    {
    public:
        // PointerGestureRecognizer.ButtonsProperty (default ButtonsMask.Primary).
        static const maui::core::bindable_property<buttons_mask>& buttons_property();

        // The mouse buttons that should trigger the pointer events (PointerGestureRecognizer.Buttons).
        [[nodiscard]] buttons_mask buttons() const
        {
            return buttons_.get();
        }
        void set_buttons(buttons_mask value)
        {
            buttons_.set(value);
        }

        // Raised when the pointer enters / exits / moves within the view, initiates a press, or
        // releases a previous press (PointerGestureRecognizer.PointerEntered/Exited/Moved/Pressed/
        // Released).
        maui::core::event<pointer_event_args> pointer_entered;
        maui::core::event<pointer_event_args> pointer_exited;
        maui::core::event<pointer_event_args> pointer_moved;
        maui::core::event<pointer_event_args> pointer_pressed;
        maui::core::event<pointer_event_args> pointer_released;

        // The platform-bridge inbound channel (PointerGestureRecognizer.SendPointerEntered/…). The
        // sender is the attached view; the port's events carry args only (no sender).
        void send_pointer_entered(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                  buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            pointer_entered.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_exited(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                 buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            pointer_exited.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_moved(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            pointer_moved.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_pressed(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                  buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            pointer_pressed.raise(pointer_event_args{.button = button, .position = position});
        }
        void send_pointer_released(element& sender, std::optional<maui::graphics::point> position = std::nullopt,
                                   buttons_mask button = buttons_mask::primary)
        {
            (void)sender;
            pointer_released.raise(pointer_event_args{.button = button, .position = position});
        }

    private:
        maui::core::property<buttons_mask> buttons_{*this, buttons_property()};
    };
} // namespace maui::controls
