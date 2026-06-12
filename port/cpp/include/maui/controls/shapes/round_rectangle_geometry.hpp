#pragma once
// maui::controls::shapes::round_rectangle_geometry  <=
//   Microsoft.Maui.Controls.Shapes.RoundRectangleGeometry
//
// A rounded rectangle given a rect + per-corner radii. Ported from RoundRectangleGeometry.cs:
// append_path is its AppendPath override (AppendRoundedRectangle with the four corner radii —
// note C#'s GeometryGroup base / UpdateGeometry children rebuild exists only for the Windows
// platform path, which consumes the Children; rendering everywhere else goes through this
// AppendPath, which ignores the children). The port derives geometry_group for surface fidelity
// (Children/FillRule are reachable) but, like C#'s AppendPath override, draws the rounded rect
// directly; the UpdateGeometry corner-ellipse decomposition is not modeled (documented).

#include "maui/controls/shapes/geometry_group.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls::shapes
{
    class round_rectangle_geometry : public geometry_group
    {
    public:
        round_rectangle_geometry() = default;
        // C# RoundRectangleGeometry(CornerRadius cornerRadius, Rect rect).
        round_rectangle_geometry(const maui::graphics::corner_radius& corner_radius, const maui::graphics::rect& rect)
            : rect_(rect), corner_radius_(corner_radius)
        {
        }

        [[nodiscard]] const maui::graphics::rect& rect() const
        {
            return rect_;
        }
        void set_rect(const maui::graphics::rect& value)
        {
            rect_ = value;
        }
        [[nodiscard]] const maui::graphics::corner_radius& corner_radius() const
        {
            return corner_radius_;
        }
        void set_corner_radius(const maui::graphics::corner_radius& value)
        {
            corner_radius_ = value;
        }

        void append_path(maui::graphics::path_f& path) const override
        {
            path.append_rounded_rectangle(
                static_cast<float>(rect_.x), static_cast<float>(rect_.y), static_cast<float>(rect_.width),
                static_cast<float>(rect_.height), static_cast<float>(corner_radius_.top_left),
                static_cast<float>(corner_radius_.top_right), static_cast<float>(corner_radius_.bottom_left),
                static_cast<float>(corner_radius_.bottom_right));
        }

    private:
        maui::graphics::rect rect_;
        maui::graphics::corner_radius corner_radius_;
    };
} // namespace maui::controls::shapes
