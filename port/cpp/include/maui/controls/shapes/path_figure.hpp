#pragma once
// maui::controls::shapes::path_figure  <=  Microsoft.Maui.Controls.Shapes.PathFigure
//   (+ PathFigureCollection.cs — the figures vector)
//
// A subsection of a path geometry: a start point plus a run of segments, optionally closed/filled.
// Ported member for member from PathFigure.cs; the BindableObject/IAnimatable + the segment
// resubscription machinery is collapsed to plain members (the geometry.hpp port collapse).
//
// Ownership: a path_geometry owns its figures, a figure owns its segments (shared_ptr vectors).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/shapes/path_segment.hpp"
#include "maui/graphics/point.hpp"

namespace maui::controls::shapes
{
    class path_figure
    {
    public:
        path_figure() = default;

        // C# PathFigure.StartPoint (default (0, 0)).
        [[nodiscard]] const maui::graphics::point& start_point() const
        {
            return start_point_;
        }
        void set_start_point(const maui::graphics::point& value)
        {
            start_point_ = value;
        }

        // C# PathFigure.Segments.
        [[nodiscard]] const path_segment_collection& segments() const
        {
            return segments_;
        }
        [[nodiscard]] path_segment_collection& segments()
        {
            return segments_;
        }
        void set_segments(path_segment_collection value)
        {
            segments_ = std::move(value);
        }

        // C# PathFigure.IsClosed (default false) — connect the last segment back to the start.
        [[nodiscard]] bool is_closed() const
        {
            return is_closed_;
        }
        void set_is_closed(bool value)
        {
            is_closed_ = value;
        }

        // C# PathFigure.IsFilled (default true).
        [[nodiscard]] bool is_filled() const
        {
            return is_filled_;
        }
        void set_is_filled(bool value)
        {
            is_filled_ = value;
        }

    private:
        maui::graphics::point start_point_;
        path_segment_collection segments_;
        bool is_closed_ = false;
        bool is_filled_ = true;
    };

    // Microsoft.Maui.Controls.Shapes.PathFigureCollection.
    using path_figure_collection = std::vector<std::shared_ptr<path_figure>>;
} // namespace maui::controls::shapes
