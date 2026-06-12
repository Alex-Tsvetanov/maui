#pragma once
// maui::controls::shapes::ellipse  <=  Microsoft.Maui.Controls.Shapes.Ellipse
//
// A shape drawing an ellipse/circle. Ported from Ellipse.cs: Aspect defaults to Fill
// (path_aspect::stretch — the ctor override); GetPath appends an ellipse inset by half the stroke
// thickness.

#include "maui/controls/shapes/shape.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class ellipse final : public shape
    {
    public:
        ellipse()
        {
            this->set_style_target_type<ellipse>();
            set_aspect(maui::core::path_aspect::stretch); // C# ctor: Aspect = Stretch.Fill
        }

        [[nodiscard]] maui::graphics::path_f get_path() const override
        {
            const double width = width_for_path_computation();
            const double height = height_for_path_computation();

            maui::graphics::path_f path;

            const auto x = static_cast<float>(stroke_thickness() / 2);
            const auto y = static_cast<float>(stroke_thickness() / 2);
            const auto w = static_cast<float>(width - stroke_thickness());
            const auto h = static_cast<float>(height - stroke_thickness());

            path.append_ellipse(x, y, w, h);
            return path;
        }
    };
} // namespace maui::controls::shapes
