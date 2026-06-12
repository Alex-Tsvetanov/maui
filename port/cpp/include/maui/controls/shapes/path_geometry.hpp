#pragma once
// maui::controls::shapes::path_geometry  <=  Microsoft.Maui.Controls.Shapes.PathGeometry
//
// A complex geometry composed of path figures. Ported from PathGeometry.cs: append_path walks every
// figure (MoveTo its start point, then the per-segment dispatch — line/polyline/cubic/poly-cubic/
// quad/poly-quad direct, arcs through geometry_helper's flatten_arc with tolerance 1 — then Close
// for a closed figure). FillRule defaults to EvenOdd.
//
// PORT COLLAPSE (geometry.hpp note): the figure/segment PropertyChanged + the
// InvalidatePathGeometryRequested event resubscription disappear — re-set the owning path's data to
// retrigger. Ownership: the geometry owns its figures (shared_ptr vector).
//
// Out-of-line definitions: src/controls/shapes/path_geometry.cpp.

#include <utility>

#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/path_figure.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class path_geometry final : public geometry
    {
    public:
        path_geometry() = default;
        // C# PathGeometry(PathFigureCollection figures) / (figures, fillRule).
        explicit path_geometry(path_figure_collection figures) : figures_(std::move(figures))
        {
        }
        path_geometry(path_figure_collection figures, shapes::fill_rule fill_rule)
            : figures_(std::move(figures)), fill_rule_(fill_rule)
        {
        }

        [[nodiscard]] const path_figure_collection& figures() const
        {
            return figures_;
        }
        [[nodiscard]] path_figure_collection& figures()
        {
            return figures_;
        }
        void set_figures(path_figure_collection value)
        {
            figures_ = std::move(value);
        }

        // C# PathGeometry.FillRule (default EvenOdd).
        [[nodiscard]] shapes::fill_rule fill_rule() const
        {
            return fill_rule_;
        }
        void set_fill_rule(shapes::fill_rule value)
        {
            fill_rule_ = value;
        }

        void append_path(maui::graphics::path_f& path) const override;

    private:
        path_figure_collection figures_;
        shapes::fill_rule fill_rule_ = shapes::fill_rule::even_odd;
    };
} // namespace maui::controls::shapes
