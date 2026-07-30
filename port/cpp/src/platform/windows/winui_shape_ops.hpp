#pragma once
// winui_shape_ops — the maui::graphics::path_f -> WinUI PathGeometry walk shared by every Windows
// handler that hosts a native vector shape: shape_view_handler.cpp (a Microsoft.UI.Xaml.Shapes.Path
// fed by ShapeDrawable's own path) and border_handler.cpp (the stroke Path fed by BorderStroke's
// shape). Both used to carry their own copy of this walk; border_handler.cpp's copy was the stale one
// (pre-flattened to move/line/close, dropping real quad/cubic/arc segments — see its old call site's
// history). This header is the ONE walk, fixed once in shape_view_handler.cpp and moved here
// verbatim, that both now call.
//
// Ported from GraphicsExtensions.AsCanvasGeometry (src/Graphics/src/Graphics/Platforms/Windows/
// GraphicsExtensions.cs:119-248): Line -> a running PolyLineSegment (AddLine), Quad ->
// QuadraticBezierSegment (AddQuadraticBezier, :165-173), Cubic -> BezierSegment (AddCubicBezier,
// :174-183), Arc -> ArcSegment (AddArc, :184-232, via GeometryUtil's EllipseAngleToPoint/GetSweep,
// src/Graphics/src/Graphics/GeometryUtil.cs). No pre-flattening: MAUI's Windows renderer never
// flattens a path before handing it to Win2D/WinUI, so this walk emits real curve segments.
//
// The one real difference between the two callers is FillRule. shape_view_handler.cpp always has a
// winding_mode (i_shape_view::fill_winding()) and needs PathGeometry.FillRule set from it — that
// control's own oracle carries one. border_handler.cpp has NO winding-mode surface at all
// (i_border_stroke exposes none) and must leave FillRule at WinUI's own default (EvenOdd) — see
// border_handler.cpp's call site for the full justification. `winding` is therefore optional:
// nullopt (the default) means "do not touch FillRule", matching border's oracle exactly rather than
// silently handing it a fill rule it never had.

#include <winrt/Microsoft.UI.Xaml.Media.h>

#include <optional>

#include "maui/graphics/winding_mode.hpp"

namespace maui::graphics
{
    class path_f;
} // namespace maui::graphics

namespace maui::platform::windows
{
    // Builds a WinUI PathGeometry from `path`, operation by operation (move/line/quad/cubic/arc/
    // close) — see the header comment above for the oracle mapping and the FillRule split. Pass a
    // winding_mode to set PathGeometry.FillRule from it (shape_view_handler.cpp); omit it (leave at
    // nullopt) to leave FillRule at WinUI's own default, untouched (border_handler.cpp).
    [[nodiscard]] winrt::Microsoft::UI::Xaml::Media::PathGeometry build_path_geometry(
        const maui::graphics::path_f& path, std::optional<maui::graphics::winding_mode> winding = std::nullopt);
} // namespace maui::platform::windows
