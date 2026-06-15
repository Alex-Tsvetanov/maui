#pragma once
// maui::controls::slider  <=  Microsoft.Maui.Controls.Slider
//
// A horizontal bar the user slides to select a value from a continuous range. Ported from
// src/Controls/src/Core/Slider/Slider.cs.
//
// Clamping semantics (the SliderUnitTests oracle): Value coerces through Clamp(Minimum, Maximum) —
// where max wins when the range is inverted (Graphics NumericExtensions.Clamp) — and a Minimum/Maximum
// change RECOERCES Value: the value the user actually requested is remembered (_requestedValue), so
// widening the range back restores it (set-order independence: any of the six Min/Max/Value orders
// yields the same result). Min/Max themselves are unvalidated (a temporarily inverted range is legal).
//
// Drag channel: send_drag_started/send_drag_completed (the ISliderController.SendDragStarted/Completed
// trigger semantics — IsEnabled-gated, command before event); the developer-facing events are
// drag_started/drag_completed, with drag_started_command/drag_completed_command as plain callables
// (the button `command` convention — the port has no ICommand abstraction).
//
// ThumbImageSource (the async image-service fetch → native thumb swap) and the iOS UpdateOnTap platform
// configuration (the tap-to-set gesture) are BOTH ported — see slider_handler.hpp + the
// thumb_image_source / update_on_tap surface below (UpdateOnTap rides the platform-spec store via
// i_ios_slider_specifics).

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_ios_slider_specifics.hpp" // --- platform configuration (UpdateOnTap) ---
#include "maui/core/i_slider.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class slider : public view<maui::core::i_slider>, public maui::core::i_ios_slider_specifics
    {
    public:
        slider()
        {
            this->set_style_target_type<slider>();
        }

        // Slider(double min, double max, double val): min must be strictly below max; the larger bound
        // is applied first so the intermediate state stays valid; val clamps into [min, max].
        slider(double min, double max, double val) : slider()
        {
            if (min >= max)
            {
                throw std::out_of_range("slider: min must be less than max");
            }
            if (max > minimum())
            {
                set_maximum(max);
                set_minimum(min);
            }
            else
            {
                set_minimum(min);
                set_maximum(max);
            }
            set_value(clamp(val, min, max));
        }

        // Shared bindable-property descriptors (one instance per type, like Slider.*Property).
        static const maui::core::bindable_property<double>& minimum_property();
        static const maui::core::bindable_property<double>& maximum_property();
        static const maui::core::bindable_property<double>& value_property();
        static const maui::core::bindable_property<maui::graphics::color>& minimum_track_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& maximum_track_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& thumb_color_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>&
        thumb_image_source_property();

        // ---- i_range (the mutable Value doubles as the developer setter, as in C#) ----
        [[nodiscard]] double minimum() const override
        {
            return minimum_.get();
        }
        void set_minimum(double value)
        {
            minimum_.set(value);
        }
        [[nodiscard]] double maximum() const override
        {
            return maximum_.get();
        }
        void set_maximum(double value)
        {
            maximum_.set(value);
        }
        [[nodiscard]] double value() const override
        {
            return value_.get();
        }
        void set_value(double value) override
        {
            value_.set(value);
        }

        // ---- i_slider colors ----
        [[nodiscard]] maui::graphics::color minimum_track_color() const override
        {
            return minimum_track_color_.get();
        }
        void set_minimum_track_color(maui::graphics::color value)
        {
            minimum_track_color_.set(value);
        }
        [[nodiscard]] maui::graphics::color maximum_track_color() const override
        {
            return maximum_track_color_.get();
        }
        void set_maximum_track_color(maui::graphics::color value)
        {
            maximum_track_color_.set(value);
        }
        [[nodiscard]] maui::graphics::color thumb_color() const override
        {
            return thumb_color_.get();
        }
        void set_thumb_color(maui::graphics::color value)
        {
            thumb_color_.set(value);
        }

        // ---- ThumbImageSource (Slider.ThumbImageSource; default null) ----
        // The owned source (a shared_ptr the slider keeps alive) — set_thumb_image_source stores it; the
        // i_slider::thumb_image_source() override hands the handler a non-owning borrow of the live object.
        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> thumb_image_source_value() const
        {
            return thumb_image_source_.get();
        }
        void set_thumb_image_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            thumb_image_source_.set(std::move(value));
        }
        [[nodiscard]] maui::core::i_image_source* thumb_image_source() const override
        {
            return thumb_image_source_.get().get();
        }

        // ---- i_ios_slider_specifics (UpdateOnTap) ----
        // Reads the iOSSpecific.Slider.UpdateOnTap platform-spec store (defined in slider.cpp to avoid the
        // ios_specific header cycle — the entry::cursor_color pattern).
        [[nodiscard]] bool update_on_tap() const override;

        // ---- the inbound drag channel (ISliderController.SendDragStarted/SendDragCompleted) ----
        void send_drag_started() override
        {
            if (is_enabled())
            {
                if (drag_started_command)
                {
                    drag_started_command();
                }
                drag_started.raise();
            }
        }
        void send_drag_completed() override
        {
            if (is_enabled())
            {
                if (drag_completed_command)
                {
                    drag_completed_command();
                }
                drag_completed.raise();
            }
        }

        // ---- developer-facing events + commands (the outbound channel) ----
        maui::core::event<double, double> value_changed; // (old, new) — ValueChangedEventArgs
        maui::core::event<> drag_started;
        maui::core::event<> drag_completed;
        maui::core::move_only_function<void()> drag_started_command;
        maui::core::move_only_function<void()> drag_completed_command;

    private:
        // Graphics NumericExtensions.Clamp: max wins when the range is inverted.
        [[nodiscard]] static double clamp(double self, double min, double max)
        {
            if (max < min)
            {
                return max;
            }
            if (self < min)
            {
                return min;
            }
            if (self > max)
            {
                return max;
            }
            return self;
        }

        // Slider.RecoerceValue: re-run the Value coercion after a range change — restoring the value
        // the user originally requested when it fits the new range.
        void recoerce_value()
        {
            is_recoercing_ = true;
            if (user_set_value_)
            {
                value_.set(requested_value_);
            }
            else
            {
                value_.set(clamp(value_.get(), minimum(), maximum()));
            }
            is_recoercing_ = false;
        }

        // The descriptors' coercion/changed callbacks (slider.cpp) reach the recoercion state above.
        friend struct slider_descriptor_access;

        maui::core::property<double> minimum_{*this, minimum_property()};
        maui::core::property<double> maximum_{*this, maximum_property()};
        maui::core::property<double> value_{*this, value_property()};
        maui::core::property<maui::graphics::color> minimum_track_color_{*this, minimum_track_color_property()};
        maui::core::property<maui::graphics::color> maximum_track_color_{*this, maximum_track_color_property()};
        maui::core::property<maui::graphics::color> thumb_color_{*this, thumb_color_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> thumb_image_source_{
            *this, thumb_image_source_property()};
        double requested_value_ = 0;  // the value the user asked for, before clamping
        bool user_set_value_ = false; // whether Value was ever explicitly set (vs recoercion)
        bool is_recoercing_ = false;
    };
} // namespace maui::controls
