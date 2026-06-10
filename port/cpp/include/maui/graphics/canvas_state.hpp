#pragma once
// maui::graphics::canvas_state  <=  Microsoft.Maui.Graphics.CanvasState
//
// The per-save-level canvas state: stroke dash pattern/offset, stroke size, and the tracked
// transformation matrix with its derived scales. Ported from src/Graphics/src/Graphics/CanvasState.cs.
// abstract_canvas (the save/restore stack base, abstract_canvas.hpp) stacks copies of a derived
// state type — the C# protected-ctor + ICanvasStateService pattern collapses to plain C++
// default/copy construction (RAII replaces IDisposable; states own nothing native here).
//
// Out-of-line definitions live in src/graphics/canvas_state.cpp.

#include <vector>

#include "maui/graphics/matrix3x2.hpp"

namespace maui::graphics
{
    class canvas_state
    {
    public:
        canvas_state() = default; // C# protected CanvasState() — public here (see header note)
        canvas_state(const canvas_state& prototype) = default; // C# protected CanvasState(prototype)
        canvas_state(canvas_state&&) = default;
        canvas_state& operator=(const canvas_state&) = default;
        canvas_state& operator=(canvas_state&&) = default;
        virtual ~canvas_state() = default;

        // C# CanvasState.StrokeDashPattern { get; set; } — empty stands in for C# null.
        [[nodiscard]] const std::vector<float>& stroke_dash_pattern() const;
        void set_stroke_dash_pattern(std::vector<float> value);

        // C# CanvasState.StrokeDashOffset { get; set; } — default 1.
        [[nodiscard]] float stroke_dash_offset() const;
        void set_stroke_dash_offset(float value);

        // C# CanvasState.StrokeSize { get; set; } — default 1.
        [[nodiscard]] float stroke_size() const;
        void set_stroke_size(float value);

        // C# CanvasState.Transform { get; set; } — setting re-derives the scales and raises
        // transform_changed() (no-op when the value is unchanged).
        [[nodiscard]] const matrix3x2& transform() const;
        void set_transform(const matrix3x2& value);

        // C# CanvasState.Scale / ScaleX / ScaleY — derived from the transform (DeconstructScales).
        [[nodiscard]] float scale() const;
        [[nodiscard]] float scale_x() const;
        [[nodiscard]] float scale_y() const;

    protected:
        // C# CanvasState.TransformChanged — for derived states that track the transform natively.
        virtual void transform_changed();

        // C# CanvasState.GetLengthScale(Matrix3x2) (protected static helper).
        [[nodiscard]] static float get_length_scale_of(const matrix3x2& matrix);

    private:
        std::vector<float> stroke_dash_pattern_;
        float stroke_dash_offset_ = 1;
        float stroke_size_ = 1;

        matrix3x2 transform_ = matrix3x2::identity();
        float scale_ = 1;
        float scale_x_ = 1;
        float scale_y_ = 1;
    };
} // namespace maui::graphics
