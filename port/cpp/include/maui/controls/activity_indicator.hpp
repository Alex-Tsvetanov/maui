#pragma once
// maui::controls::activity_indicator  <=  Microsoft.Maui.Controls.ActivityIndicator
//
// A visual clue that something is ongoing, without progress information. Ported from
// src/Controls/src/Core/ActivityIndicator/ActivityIndicator.cs: IsRunning (whether the spinner is
// visible and animating) and Color (ColorElement.ColorProperty).

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class activity_indicator : public view<maui::core::i_activity_indicator>
    {
    public:
        activity_indicator()
        {
            this->set_style_target_type<activity_indicator>();
        }

        // Shared bindable-property descriptors (one instance per type, like ActivityIndicator.*Property).
        static const maui::core::bindable_property<bool>& is_running_property();
        static const maui::core::bindable_property<maui::graphics::color>& color_property();

        // ---- i_activity_indicator (read by the handler's mapper) ----
        [[nodiscard]] bool is_running() const override
        {
            return is_running_.get();
        }
        void set_is_running(bool value)
        {
            is_running_.set(value);
        }
        [[nodiscard]] maui::graphics::color color() const override
        {
            return color_.get();
        }
        void set_color(maui::graphics::color value)
        {
            color_.set(value);
        }

    private:
        maui::core::property<bool> is_running_{*this, is_running_property()};
        maui::core::property<maui::graphics::color> color_{*this, color_property()};
    };
} // namespace maui::controls
