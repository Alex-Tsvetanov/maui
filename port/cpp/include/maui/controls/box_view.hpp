#pragma once
// maui::controls::box_view  <=  Microsoft.Maui.Controls.BoxView
//
// A view drawing a solid colored (optionally rounded) rectangle — over the shape machinery: BoxView
// is its own IShapeView AND its own IShape (Shape => this), rendered by the shared
// shape_view_handler. Ported from BoxView.cs:
//   - Color (ColorElement.ColorProperty, default null — the frame border_color optional pattern) and
//     CornerRadius (CornerElement.CornerRadiusProperty);
//   - IShapeView.Fill => Color?.AsPaint() (a solid paint materialized from the color; the control
//     owns the cached paint), Aspect => None, the stroke surface all empty/zero;
//   - IShape.PathForBounds: a rounded rectangle over the given bounds with the four corner radii;
//   - OnMeasure: the default 40x40 size request (resolved against the explicit requests, the
//     view<>::measure tail).
// The "color"/"corner_radius" property keys ride shape_view_handler's table (BoxView.
// OnPropertyChanged → UpdateValue(IShapeView.Shape)).

#include <memory>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    class box_view : public view<maui::core::i_shape_view>, public maui::graphics::i_shape
    {
    public:
        box_view()
        {
            this->set_style_target_type<box_view>();
        }

        // Shared bindable-property descriptors (BoxView.ColorProperty / CornerRadiusProperty).
        static const maui::core::bindable_property<maui::graphics::color>& color_property();
        static const maui::core::bindable_property<maui::graphics::corner_radius>& corner_radius_property();

        // ---- Color (unset = C# null; the frame border_color optional pattern) ----
        [[nodiscard]] bool has_color() const
        {
            return color_.is_set();
        }
        [[nodiscard]] maui::graphics::color color() const
        {
            return color_.get();
        }
        void set_color(maui::graphics::color value)
        {
            color_.set(value);
        }

        // ---- CornerRadius ----
        [[nodiscard]] maui::graphics::corner_radius corner_radius() const
        {
            return corner_radius_.get();
        }
        void set_corner_radius(maui::graphics::corner_radius value)
        {
            corner_radius_.set(value);
        }

        // ---- i_shape_view (the explicit C# implementations) ----
        [[nodiscard]] maui::graphics::i_shape* shape() const override
        {
            return shape_self_;
        }
        [[nodiscard]] maui::core::path_aspect aspect() const override
        {
            return maui::core::path_aspect::none;
        }
        // C# Fill => Color?.AsPaint(): a solid paint over the color, owned by the control (the cache
        // refreshes per read so it always carries the current color); null while Color is unset.
        [[nodiscard]] maui::graphics::paint* fill() const override;

        // ---- i_stroke (all empty/zero in C#) ----
        [[nodiscard]] maui::graphics::paint* stroke() const override
        {
            return nullptr;
        }
        [[nodiscard]] double stroke_thickness() const override
        {
            return 0;
        }
        [[nodiscard]] maui::graphics::line_cap stroke_line_cap() const override
        {
            return maui::graphics::line_cap::butt;
        }
        [[nodiscard]] maui::graphics::line_join stroke_line_join() const override
        {
            return maui::graphics::line_join::miter;
        }
        [[nodiscard]] std::vector<float> stroke_dash_pattern() const override
        {
            return {};
        }
        [[nodiscard]] float stroke_dash_offset() const override
        {
            return 0;
        }
        [[nodiscard]] float stroke_miter_limit() const override
        {
            return 0;
        }

        // ---- i_shape (C# IShape.PathForBounds) ----
        [[nodiscard]] maui::graphics::path_f path_for_bounds(const maui::graphics::rect& bounds) const override;

        // C# BoxView.OnMeasure: the default 40x40 request (the view<>::measure resolve tail applies
        // WidthRequest/HeightRequest and the min/max clamps over it).
        maui::graphics::size measure(double width_constraint, double height_constraint) override;

    private:
        maui::core::property<maui::graphics::color> color_{*this, color_property()};
        maui::core::property<maui::graphics::corner_radius> corner_radius_{*this, corner_radius_property()};
        maui::graphics::i_shape* shape_self_ = this; // the control's own i_shape face (Shape => this)
        // The owned Fill paint cache (header note); mutable — refreshed inside the const fill() read.
        mutable std::shared_ptr<maui::graphics::solid_paint> fill_cache_;
    };
} // namespace maui::controls
