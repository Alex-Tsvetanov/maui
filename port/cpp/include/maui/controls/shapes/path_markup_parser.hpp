#pragma once
// maui::controls::shapes — the path markup / point list string parsers  <=
//   Microsoft.Maui.Controls.Shapes.{PathGeometryConverter, PathFigureCollectionConverter,
//   PointCollectionConverter}
//
// Free parse functions over the geometry model (PROFILE §6: explicit registration replaces
// TypeConverter discovery — a XAML converter wave registers these callables later; nothing under
// maui/xaml is touched here). The C# converters' parse halves are ported 1:1:
//   - parse_path_figure_collection ⇐ PathFigureCollectionConverter.ParseStringToPathFigureCollection
//     (the WPF abbreviated-geometry "M0,0 L10,10 …" grammar: M/L/H/V/C/S/Q/T/A/Z + relative
//     lower-case forms, the leading F0|F1 fill-rule token accepted and skipped, smooth-curve
//     reflection, Infinity/NaN literals). Errors throw std::invalid_argument (C# FormatException).
//   - parse_path_geometry ⇐ PathGeometryConverter.ConvertFrom — a fresh path_geometry whose figures
//     are parsed from the string (empty string/figures for an empty/absent input, like the C# null).
//     Returns a shared_ptr (PROFILE §8): path_geometry is a bindable_object (2026-07 XAML-registration
//     fix, geometry.hpp) and therefore non-copyable/non-movable, so it can no longer be returned by
//     value — callers already wrapped the old by-value result in make_shared<path_geometry>(...)
//     anyway (path_gallery_page / update_path_data_page), so this is the natural ownership shape.
//   - parse_point_collection ⇐ PointCollectionConverter.ConvertFrom — "x1,y1 x2,y2 …" (spaces and
//     commas both separate). Errors throw std::invalid_argument (C# InvalidOperationException).
//
// Out-of-line definitions: src/controls/shapes/path_markup_parser.cpp. The C# ConvertTo (serialize)
// halves are deferred with the XAML wave (documented, not stubbed).

#include <memory>
#include <string_view>

#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_segment.hpp"

namespace maui::controls::shapes
{
    // C# PathFigureCollectionConverter.ParseStringToPathFigureCollection(collection, pathString).
    void parse_path_figure_collection(path_figure_collection& figures, std::string_view path_string);

    // C# PathGeometryConverter.ConvertFrom (the parse direction).
    [[nodiscard]] std::shared_ptr<path_geometry> parse_path_geometry(std::string_view path_string);

    // C# PointCollectionConverter.ConvertFrom (the parse direction).
    [[nodiscard]] point_collection parse_point_collection(std::string_view points_string);
} // namespace maui::controls::shapes
