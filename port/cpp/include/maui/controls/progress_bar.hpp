#pragma once
// maui::controls::progress_bar  <=  Microsoft.Maui.Controls.ProgressBar
//
// Displays progress as a partially filled horizontal bar. Ported from
// src/Controls/src/Core/ProgressBar/ProgressBar.cs.
//
// Progress coerces through Clamp(0, 1) (values outside the range are clamped — the ProgressBarTests
// oracle); ProgressColor is the fill color.
//
// Not ported (deferred, documented in port/STATUS.md): ProgressTo(value, length, easing) — the
// animation-driven progress sweep needs the Animation/Easing subsystem (this.Animate), which the port
// does not have yet. Set `progress` directly until the animations unit lands.

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class progress_bar : public view<maui::core::i_progress>
    {
    public:
        progress_bar()
        {
            this->set_style_target_type<progress_bar>();
        }

        // Shared bindable-property descriptors (one instance per type, like ProgressBar.*Property).
        static const maui::core::bindable_property<double>& progress_property();
        static const maui::core::bindable_property<maui::graphics::color>& progress_color_property();

        // ---- i_progress (read by the handler's mapper) ----
        [[nodiscard]] double progress() const override
        {
            return progress_.get();
        }
        void set_progress(double value)
        {
            progress_.set(value);
        }
        [[nodiscard]] maui::graphics::color progress_color() const override
        {
            return progress_color_.get();
        }
        void set_progress_color(maui::graphics::color value)
        {
            progress_color_.set(value);
        }

    private:
        maui::core::property<double> progress_{*this, progress_property()};
        maui::core::property<maui::graphics::color> progress_color_{*this, progress_color_property()};
    };
} // namespace maui::controls
