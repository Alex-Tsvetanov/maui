// maui::core::shape_drawable — out-of-line definitions. Ported from ShapeDrawable.cs: fill first
// (transparent fill color staged, the path clipped with the winding mode, the fill paint over the
// dirty rect), then the stroke on top (the full IStroke surface staged + SetFillPaint over the
// path's bounds so gradient strokes map to the shape geometry). The stroke color is the paint's
// background_color projection (C# stroke.ToColor() — the solid/first-gradient-stop color; the
// border_stroke_spec precedent).

#include "maui/core/shape_drawable.hpp"

#include "maui/core/i_shape_view.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::core
{
    void shape_drawable::draw(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        const maui::graphics::rect_f& rect = dirty_rect;

        const maui::graphics::i_shape* shape = shape_view_ != nullptr ? shape_view_->shape() : nullptr;
        if (shape == nullptr)
        {
            return;
        }

        maui::graphics::path_f path =
            shape->path_for_bounds(maui::graphics::rect{rect.x, rect.y, rect.width, rect.height});

        // C# ApplyTransform.
        if (render_transform_.has_value())
        {
            path.transform(*render_transform_);
        }

        // Draw the fill first, then the stroke on top so the stroke stays fully visible.
        draw_fill_path(canvas, rect, path);
        draw_stroke_path(canvas, rect, path);
    }

    void shape_drawable::draw_stroke_path(maui::graphics::i_canvas& canvas,
                                          const maui::graphics::rect_f& /*dirty_rect*/,
                                          const maui::graphics::path_f& path) const
    {
        if (shape_view_ == nullptr || shape_view_->shape() == nullptr || shape_view_->stroke_thickness() <= 0 ||
            shape_view_->stroke() == nullptr)
        {
            return;
        }

        canvas.save_state();

        canvas.set_stroke_size(static_cast<float>(shape_view_->stroke_thickness()));

        const maui::graphics::paint* stroke = shape_view_->stroke();
        canvas.set_stroke_color(stroke->background_color()); // C# stroke.ToColor()

        canvas.set_stroke_line_cap(shape_view_->stroke_line_cap());
        canvas.set_stroke_line_join(shape_view_->stroke_line_join());
        canvas.set_stroke_dash_pattern(shape_view_->stroke_dash_pattern());
        canvas.set_stroke_dash_offset(shape_view_->stroke_dash_offset());
        canvas.set_miter_limit(shape_view_->stroke_miter_limit());

        // C#: SetFillPaint(stroke, path.Bounds) configures gradient state for the stroke render,
        // mapped to the shape geometry rather than the whole dirty rect.
        canvas.set_fill_paint(stroke, path.bounds());
        canvas.draw_path(path);

        canvas.restore_state();
    }

    void shape_drawable::draw_fill_path(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect,
                                        const maui::graphics::path_f& path) const
    {
        if (shape_view_ == nullptr || shape_view_->shape() == nullptr)
        {
            return;
        }

        canvas.save_state();

        canvas.set_fill_color(maui::graphics::colors::transparent);

        // C# ClipPath(path, WindingMode).
        canvas.clip_path(path, winding_mode_);

        // C#: Fill ?? Background.
        const maui::graphics::paint* fill_paint = shape_view_->fill();
        if (fill_paint == nullptr)
        {
            fill_paint = shape_view_->background();
        }
        if (fill_paint != nullptr)
        {
            canvas.set_fill_paint(fill_paint, dirty_rect);
        }

        // C# canvas.FillPath(path) — the parameterless extension fills NonZero.
        canvas.fill_path(path, maui::graphics::winding_mode::non_zero);

        canvas.restore_state();
    }
} // namespace maui::core
