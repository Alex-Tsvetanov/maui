#pragma once
// maui::controls::shapes::rectangle  <=  Microsoft.Maui.Controls.Shapes.Rectangle
//
// A shape drawing a rectangle, optionally rounded via RadiusX/RadiusY. Ported from Rectangle.cs:
// Aspect defaults to Fill (path_aspect::stretch — the ctor override), GetPath insets by half the
// stroke thickness and appends a (rounded) rectangle with max(RadiusX, RadiusY) (the C# TODO keeps
// one radius). The RadiusX/RadiusY property keys ride shape_view_handler's absorbed sub-handler
// table ("radius_x"/"radius_y" → InvalidateShape — Rectangle.OnPropertyChanged's UpdateValue(Shape)).

#include <algorithm>

#include "maui/controls/shapes/shape.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class rectangle final : public shape
    {
    public:
        rectangle()
        {
            this->set_style_target_type<rectangle>();
            set_aspect(maui::core::path_aspect::stretch); // C# ctor: Aspect = Stretch.Fill
        }

        static const maui::core::bindable_property<double>& radius_x_property();
        static const maui::core::bindable_property<double>& radius_y_property();

        [[nodiscard]] double radius_x() const
        {
            return radius_x_.get();
        }
        void set_radius_x(double value)
        {
            radius_x_.set(value);
        }
        [[nodiscard]] double radius_y() const
        {
            return radius_y_.get();
        }
        void set_radius_y(double value)
        {
            radius_y_.set(value);
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
            const auto corner_radius = static_cast<float>(std::max(radius_x(), radius_y()));

            // C#: AppendRoundedRectangle slashes the corners even for cornerRadius == 0, so the
            // plain rectangle is appended in that case.
            if (corner_radius == 0)
            {
                path.append_rectangle(x, y, w, h);
            }
            else
            {
                path.append_rounded_rectangle(x, y, w, h, corner_radius);
            }

            return path;
        }

    private:
        maui::core::property<double> radius_x_{*this, radius_x_property()};
        maui::core::property<double> radius_y_{*this, radius_y_property()};
    };
} // namespace maui::controls::shapes
