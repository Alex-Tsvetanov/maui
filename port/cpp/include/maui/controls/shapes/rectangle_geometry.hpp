#pragma once
// maui::controls::shapes::rectangle_geometry  <=  Microsoft.Maui.Controls.Shapes.RectangleGeometry
//
// The geometry of an axis-aligned rectangle. Ported from RectangleGeometry.cs; append_path is its
// AppendPath (AppendRectangle over the stored rect).

#include "maui/controls/shapes/geometry.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls::shapes
{
    class rectangle_geometry : public geometry
    {
    public:
        rectangle_geometry() = default;
        // C# RectangleGeometry(Rect rect).
        explicit rectangle_geometry(const maui::graphics::rect& rect) : rect_(rect)
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

        void append_path(maui::graphics::path_f& path) const override
        {
            path.append_rectangle(static_cast<float>(rect_.x), static_cast<float>(rect_.y),
                                  static_cast<float>(rect_.width), static_cast<float>(rect_.height));
        }

    private:
        maui::graphics::rect rect_;
    };
} // namespace maui::controls::shapes
