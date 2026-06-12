#pragma once
// maui::core::i_shape_view  <=  Microsoft.Maui.IShapeView
//
// The virtual-view contract for a view that renders a shape: the shape definition + the aspect +
// the fill brush, over the i_stroke outline surface. Ported from src/Core/src/Core/IShapeView.cs
// (IShapeView : IView, IStroke).
//
// shape() / fill() return non-owning borrows (the control owns them via shared_ptr — the i_view
// clip/background ownership rule); null = no shape / no fill.
//
// PORT EXTENSION (documented): C# pushes the fill winding mode and the path render transform into
// the ShapeDrawable through per-shape SUB-handlers (PolylineHandler.MapFillRule /
// PathHandler.MapRenderTransform in src/Controls/src/Core/Handlers/Shapes/*). The port collapses
// that family onto ONE shape_view_handler (see shape_view_handler.hpp), so the two values surface
// here as defaulted virtuals instead: fill_winding() (non_zero unless a polygon/polyline maps its
// EvenOdd FillRule) and render_transform_matrix() (nullopt unless a path carries a RenderTransform).

#include <optional>

#include "maui/core/i_stroke.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/winding_mode.hpp"

namespace maui::core
{
    class i_shape_view : public i_view, public i_stroke
    {
    public:
        // C# IShapeView.Shape — the shape definition to render (null = nothing to draw).
        [[nodiscard]] virtual maui::graphics::i_shape* shape() const = 0;
        // C# IShapeView.Aspect — how the shape stretches into the layout space.
        [[nodiscard]] virtual path_aspect aspect() const = 0;
        // C# IShapeView.Fill — the brush painting the shape's interior (null = no fill).
        [[nodiscard]] virtual maui::graphics::paint* fill() const = 0;

        // PORT EXTENSION (header note): the C# sub-handler pushes, surfaced as defaulted getters.
        [[nodiscard]] virtual maui::graphics::winding_mode fill_winding() const
        {
            return maui::graphics::winding_mode::non_zero;
        }
        [[nodiscard]] virtual std::optional<maui::graphics::matrix3x2> render_transform_matrix() const
        {
            return std::nullopt;
        }
    };
} // namespace maui::core
