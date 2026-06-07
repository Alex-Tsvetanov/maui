#pragma once
// maui::controls::image  <=  Microsoft.Maui.Controls.Image (minimal: aspect only)
//
// A view that displays an image. Ported from Image.cs — a MINIMAL cut exposing ONLY the scaling mode
// (Aspect). Same API shape as the other controls: a bare-noun interface getter + method accessors backed
// by a private property<T> whose change flows through view::on_property_changed to the handler.
//
// OUT OF SCOPE this cut (deferred async image SOURCE subsystem): Source / IsLoading / IsAnimationPlaying /
// IsOpaque. The image ships aspect-only.

#include "maui/controls/view.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class image : public view<maui::core::i_image>
    {
    public:
        // Shared bindable-property descriptor (one instance per type, like Image.AspectProperty).
        static const maui::core::bindable_property<maui::core::aspect>& aspect_property();

        // ---- i_image ----
        [[nodiscard]] maui::core::aspect aspect() const override
        {
            return aspect_.get();
        }

        // ---- public setter (drives the handler via on_property_changed → update_value) ----
        void set_aspect(maui::core::aspect value)
        {
            aspect_.set(value);
        }

    private:
        maui::core::property<maui::core::aspect> aspect_{*this, aspect_property()};
    };
} // namespace maui::controls
