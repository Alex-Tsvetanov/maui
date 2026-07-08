#pragma once
// maui::controls::shapes::round_rectangle  <=  Microsoft.Maui.Controls.Shapes.RoundRectangle
//
// A shape drawing a rounded rectangle with per-corner radii. Ported from RoundRectangle.cs:
// Aspect defaults to Fill (path_aspect::stretch — the ctor override). GetPath appends a rounded
// rectangle over (0,0,w,h) with the four CornerRadius corners in order top-left, top-right,
// bottom-left, bottom-right — and, UNLIKE Rectangle.GetPath, applies NO half-stroke inset (the C#
// half-stroke inset lives instead in the IRoundRectangle InnerPath seam, out of scope here as in the
// sibling shapes). The CornerRadius property key rides shape_view_handler's absorbed sub-handler
// table ("corner_radius" → InvalidateShape — RoundRectangle.OnPropertyChanged's UpdateValue(Shape);
// the key is already present in shape_view_handler::mapper()).

#include "maui/controls/shapes/shape.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class round_rectangle final : public shape
    {
    public:
        round_rectangle()
        {
            this->set_style_target_type<round_rectangle>();
            set_aspect(maui::core::path_aspect::stretch); // C# ctor: Aspect = Stretch.Fill
        }

        static const maui::core::bindable_property<maui::graphics::corner_radius>& corner_radius_property();

        [[nodiscard]] maui::graphics::corner_radius corner_radius() const
        {
            return corner_radius_.get();
        }
        void set_corner_radius(maui::graphics::corner_radius value)
        {
            corner_radius_.set(value);
        }

        [[nodiscard]] maui::graphics::path_f get_path() const override
        {
            const double width = width_for_path_computation();
            const double height = height_for_path_computation();
            const maui::graphics::corner_radius radius = corner_radius_.get();

            // C# RoundRectangle.GetPath: AppendRoundedRectangle(0, 0, w, h, TL, TR, BL, BR) — no stroke
            // inset (the half-stroke inset is the IRoundRectangle InnerPath seam, not GetPath).
            maui::graphics::path_f path;
            path.append_rounded_rectangle(0.0F, 0.0F, static_cast<float>(width), static_cast<float>(height),
                                          static_cast<float>(radius.top_left), static_cast<float>(radius.top_right),
                                          static_cast<float>(radius.bottom_left),
                                          static_cast<float>(radius.bottom_right));
            return path;
        }

    private:
        maui::core::property<maui::graphics::corner_radius> corner_radius_{*this, corner_radius_property()};
    };
} // namespace maui::controls::shapes
