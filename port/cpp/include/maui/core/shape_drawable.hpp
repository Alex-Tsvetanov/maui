#pragma once
// maui::core::shape_drawable  <=  Microsoft.Maui.Graphics.ShapeDrawable
//   (src/Core/src/Graphics/ShapeDrawable.cs — the Core-assembly drawable that renders an IShapeView
//    through any ICanvas; the port keeps it in maui::core beside its i_shape_view dependency.)
//
// The i_drawable every shape view host draws: resolve the shape's path for the dirty rect, apply
// the render transform, fill first (clipped by the path + winding) then stroke on top. Ported
// member for member; the C# WeakReference<IShapeView> becomes a plain non-owning borrow (the
// handler owns the drawable and resets the borrow on connect/disconnect — §8 teardown).
//
// Out-of-line definitions: src/core/shape_drawable.cpp.

#include <optional>

#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::graphics
{
    class i_canvas;
    class path_f;
} // namespace maui::graphics

namespace maui::core
{
    class i_shape_view;

    class shape_drawable final : public maui::graphics::i_drawable
    {
    public:
        shape_drawable() = default;
        // C# ShapeDrawable(IShapeView? shape).
        explicit shape_drawable(const i_shape_view* shape_view) : shape_view_(shape_view)
        {
        }

        // C# ShapeDrawable.UpdateShapeView — re-point the drawable at the shape view (non-owning).
        void update_shape_view(const i_shape_view* shape_view)
        {
            shape_view_ = shape_view;
        }
        [[nodiscard]] const i_shape_view* shape_view() const
        {
            return shape_view_;
        }

        // C# ShapeDrawable.UpdateWindingMode (the fill/clip winding; default NonZero).
        void update_winding_mode(maui::graphics::winding_mode winding)
        {
            winding_mode_ = winding;
        }
        [[nodiscard]] maui::graphics::winding_mode winding_mode() const
        {
            return winding_mode_;
        }

        // C# ShapeDrawable.UpdateRenderTransform (nullopt = none).
        void update_render_transform(std::optional<maui::graphics::matrix3x2> render_transform)
        {
            render_transform_ = render_transform;
        }
        [[nodiscard]] const std::optional<maui::graphics::matrix3x2>& render_transform() const
        {
            return render_transform_;
        }

        // C# ShapeDrawable.Draw(ICanvas, RectF): PathForBounds → transform → fill → stroke.
        void draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect) override;

    private:
        // C# DrawStrokePath / DrawFillPath.
        void draw_stroke_path(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect,
                              const maui::graphics::path_f& path) const;
        void draw_fill_path(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect,
                            const maui::graphics::path_f& path) const;

        const i_shape_view* shape_view_ = nullptr; // non-owning (see header)
        maui::graphics::winding_mode winding_mode_ = maui::graphics::winding_mode::non_zero;
        std::optional<maui::graphics::matrix3x2> render_transform_;
    };
} // namespace maui::core
