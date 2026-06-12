#pragma once
// maui::controls::shapes::line  <=  Microsoft.Maui.Controls.Shapes.Line
//
// A shape drawing a straight line between (x1, y1) and (x2, y2). Ported from Line.cs (Aspect keeps
// the base None default). The coordinate property keys ride shape_view_handler's absorbed
// sub-handler table ("x1"/"y1"/"x2"/"y2" → InvalidateShape). Line is one of the C# margin-adding
// measure types (Shape.MeasureOverride's `is Line or Path or Polyline`).

#include "maui/controls/shapes/shape.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class line final : public shape
    {
    public:
        line()
        {
            this->set_style_target_type<line>();
        }
        // C# Line(double x1, double y1, double x2, double y2).
        line(double x1, double y1, double x2, double y2) : line()
        {
            set_x1(x1);
            set_y1(y1);
            set_x2(x2);
            set_y2(y2);
        }

        static const maui::core::bindable_property<double>& x1_property();
        static const maui::core::bindable_property<double>& y1_property();
        static const maui::core::bindable_property<double>& x2_property();
        static const maui::core::bindable_property<double>& y2_property();

        [[nodiscard]] double x1() const
        {
            return x1_.get();
        }
        void set_x1(double value)
        {
            x1_.set(value);
        }
        [[nodiscard]] double y1() const
        {
            return y1_.get();
        }
        void set_y1(double value)
        {
            y1_.set(value);
        }
        [[nodiscard]] double x2() const
        {
            return x2_.get();
        }
        void set_x2(double value)
        {
            x2_.set(value);
        }
        [[nodiscard]] double y2() const
        {
            return y2_.get();
        }
        void set_y2(double value)
        {
            y2_.set(value);
        }

        [[nodiscard]] maui::graphics::path_f get_path() const override
        {
            maui::graphics::path_f path;
            path.move_to(static_cast<float>(x1()), static_cast<float>(y1()));
            path.line_to(static_cast<float>(x2()), static_cast<float>(y2()));
            return path;
        }

    protected:
        [[nodiscard]] bool adds_margin_to_measure() const override
        {
            return true;
        }

    private:
        maui::core::property<double> x1_{*this, x1_property()};
        maui::core::property<double> y1_{*this, y1_property()};
        maui::core::property<double> x2_{*this, x2_property()};
        maui::core::property<double> y2_{*this, y2_property()};
    };
} // namespace maui::controls::shapes
