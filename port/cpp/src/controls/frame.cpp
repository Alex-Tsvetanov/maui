// maui::controls::frame — out-of-line definitions: the descriptors (Frame.cs defaults), the facade
// translation onto the border machinery (see frame.hpp), and the default-handler self-registration
// (the same border_handler — the facade needs no native code of its own).

#include "maui/controls/frame.hpp"

#include <memory>
#include <stdexcept>

#include "maui/core/bindable_property.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    namespace
    {
        // The hard-coded shadow C#'s Frame returns for IView.Shadow when HasShadow (Frame.cs, iOS):
        // Radius 5, Opacity 0.8, Offset (0,0), Brush.Black.
        std::shared_ptr<maui::core::shadow> make_frame_shadow()
        {
            auto value = std::make_shared<maui::core::shadow>();
            value->set_radius(5.0);
            value->set_opacity(0.8);
            value->set_color(maui::graphics::color(0.0F, 0.0F, 0.0F));
            value->set_offset({0.0, 0.0});
            return value;
        }
    } // namespace

    frame::frame() : border(padding_property())
    {
        this->set_style_target_type<frame>();
        // Facade invariants: no border until BorderColor is set (StrokeThickness 0 keeps the measure
        // inset Padding-only, the C# `BorderColor is not null ? 1 : 0` term), and the default
        // HasShadow=true materializes the canned frame shadow.
        set_stroke_thickness(0.0);
        set_shadow(make_frame_shadow());
    }

    const maui::core::bindable_property<maui::core::thickness>& frame::padding_property()
    {
        // C# Frame's IPaddingElement.PaddingDefaultValueCreator returns 20.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding",
                                                                                     maui::core::thickness(20.0)};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& frame::border_color_property()
    {
        // C# BorderElement.BorderColorProperty default is null — "unset" is tracked via is_set().
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"border_color"};
        return descriptor;
    }

    const maui::core::bindable_property<float>& frame::corner_radius_property()
    {
        // C# Frame.CornerRadiusProperty default is -1 (the "use the platform default" sentinel).
        static const maui::core::bindable_property<float> descriptor{"corner_radius", -1.0F};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& frame::has_shadow_property()
    {
        // C# Frame.HasShadowProperty default is true.
        static const maui::core::bindable_property<bool> descriptor{"has_shadow", true};
        return descriptor;
    }

    void frame::set_border_color(maui::graphics::color value)
    {
        border_color_.set(value);
        // The facade translation: the FrameRenderer's fixed 1px border in the given color.
        set_stroke(std::make_shared<maui::graphics::solid_paint>(value));
        set_stroke_thickness(1.0);
    }

    void frame::set_corner_radius(float value)
    {
        // C# CornerRadiusProperty.validateValue: value == -1 || value >= 0 (ArgumentException otherwise).
        if (value < 0.0F && value != -1.0F)
        {
            throw std::invalid_argument("frame corner_radius must be -1 or >= 0");
        }
        corner_radius_.set(value);
        if (value >= 0.0F)
        {
            set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(static_cast<double>(value)));
        }
        else
        {
            set_stroke_shape(std::make_shared<maui::graphics::shapes::rectangle>());
        }
    }

    void frame::set_has_shadow(bool value)
    {
        has_shadow_.set(value);
        set_shadow(value ? make_frame_shadow() : nullptr);
    }
} // namespace maui::controls

// Self-register the default handler for frame — the same border_handler (the facade adds no native
// code; C#'s FrameRenderer lives in the out-of-scope Compatibility layer).
MAUI_REGISTER_HANDLER(maui::controls::frame, maui::core::border_handler)
