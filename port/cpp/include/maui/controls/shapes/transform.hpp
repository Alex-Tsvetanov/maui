#pragma once
// maui::controls::shapes::transform  <=  Microsoft.Maui.Controls.Shapes.Transform
//
// The base of the Shapes transform model: a holder of the WPF-style 3x3 affine matrix the concrete
// transforms (rotate/scale/skew/translate/matrix/group/composite) compute. Ported from
// src/Controls/src/Core/Shapes/Transform.cs.
//
// PORT COLLAPSE (documented, not stubbed; the Border stroke/shape precedent): the C# family is a
// BindableObject tree — every scalar is a BindableProperty whose propertyChanged recomputes the
// cached Value, and Path resubscribes PropertyChanged to re-render. The port stores plain scalars and
// computes value() ON READ (the same matrix math, the same observable values); mutating a transform
// does not auto-invalidate a path control — re-set path::set_render_transform (or call
// path::invalidate_render_transform) to retrigger the mapper, exactly like re-setting a border's
// stroke shape.
//
// Ownership (PROFILE §8): a path control owns its transform via shared_ptr; transform_group owns its
// children via shared_ptr.

#include "maui/controls/shapes/matrix.hpp"

namespace maui::controls::shapes
{
    class transform
    {
    public:
        transform() = default;
        virtual ~transform() = default;
        transform(const transform&) = default;
        transform(transform&&) = default;
        transform& operator=(const transform&) = default;
        transform& operator=(transform&&) = default;

        // C# Transform.Value — the base holds a settable matrix (Transform itself is concrete in C#);
        // the derived transforms override value() to compute theirs from their scalars.
        [[nodiscard]] virtual matrix value() const
        {
            return value_;
        }
        void set_value(const matrix& value)
        {
            value_ = value;
        }

    private:
        matrix value_;
    };
} // namespace maui::controls::shapes
