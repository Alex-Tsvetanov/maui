// maui::graphics::canvas_state — out-of-line definitions. See canvas_state.hpp. Ported from
// src/Graphics/src/Graphics/CanvasState.cs: the dash/stroke slots plus the transform with its
// DeconstructScales-derived scale cache.

#include "maui/graphics/canvas_state.hpp"

#include <utility>
#include <vector>

#include "maui/graphics/matrix3x2.hpp"

namespace maui::graphics
{
    const std::vector<float>& canvas_state::stroke_dash_pattern() const
    {
        return stroke_dash_pattern_;
    }

    void canvas_state::set_stroke_dash_pattern(std::vector<float> value)
    {
        stroke_dash_pattern_ = std::move(value);
    }

    float canvas_state::stroke_dash_offset() const
    {
        return stroke_dash_offset_;
    }

    void canvas_state::set_stroke_dash_offset(float value)
    {
        stroke_dash_offset_ = value;
    }

    float canvas_state::stroke_size() const
    {
        return stroke_size_;
    }

    void canvas_state::set_stroke_size(float value)
    {
        stroke_size_ = value;
    }

    const matrix3x2& canvas_state::transform() const
    {
        return transform_;
    }

    void canvas_state::set_transform(const matrix3x2& value)
    {
        // C# Transform setter: no-op when unchanged; otherwise store, re-derive the scales
        // (DeconstructScales) and raise TransformChanged.
        if (transform_ == value)
        {
            return;
        }

        transform_ = value;
        deconstruct_scales(value, scale_, scale_x_, scale_y_);
        transform_changed();
    }

    float canvas_state::scale() const
    {
        return scale_;
    }

    float canvas_state::scale_x() const
    {
        return scale_x_;
    }

    float canvas_state::scale_y() const
    {
        return scale_y_;
    }

    void canvas_state::transform_changed()
    {
        // C# TransformChanged: let derived states react if needed.
    }

    float canvas_state::get_length_scale_of(const matrix3x2& matrix)
    {
        // C# GetLengthScale(matrix) => matrix.GetLengthScale().
        return get_length_scale(matrix);
    }
} // namespace maui::graphics
