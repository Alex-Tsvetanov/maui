// maui::xaml — XAML registration for the shapes control group: Path, Line, Ellipse, Rectangle,
// Polygon, Polyline  (register_xaml_groups.hpp::register_xaml_shapes).
//
// Each shape control:
//   1. types.register_type<T>("XamlElement") — the markup element name (C# PascalCase).
//   2. register_view_properties<T>(properties) — the shared IView/VisualElement attribute surface.
//   3. Fill/Stroke: registered as register_property<T, shared_ptr<brush>> lambdas that call
//      set_fill_brush / set_stroke_brush — the spec's explicit bridge path. The bindable_property
//      descriptors for fill_/stroke_ are paint-typed, but XAML authors write a brush string
//      ("Red", "#FF0000", "linear-gradient(...)"); the existing shared_ptr<brush> converter handles
//      text→brush, and the shape's bridge method pins the brush's BindingContext (shape.hpp X1 note).
//      Using register_property (not register_bindable_property) avoids a type mismatch between the
//      storage type (paint) and the value type the converter produces (brush).
//   4. Shape base properties (StrokeThickness…Aspect): register_bindable_property<T> with the shared
//      shapes::shape::xxx_property() descriptors — one instance covers the whole family.
//   5. Control-specific properties (line coordinates, rectangle radii, polygon/polyline points +
//      fill rule): register_bindable_property<T> with the concrete class's own descriptors.
//   6. Content / child sinks: none — all six shapes are leaf controls (no [ContentProperty]).
//      Path.Data and Path.RenderTransform are complex object types (shared_ptr<geometry/transform>)
//      with no text converter; they require property-element child registration which is deferred
//      until geometry/transform are registered as XAML types.
//
// Converters added here (types not already in register_standard_xaml_converters):
//   - std::vector<double>  (StrokeDashArray) — DoubleCollectionConverter: space/comma-separated
//   - maui::graphics::line_cap  (StrokeLineCap) — PenLineCap names: Flat/Butt/Round/Square
//   - maui::graphics::line_join (StrokeLineJoin) — PenLineJoin names: Miter/Round/Bevel
//   - maui::core::path_aspect   (Aspect) — C# Stretch names: None/Fill/Uniform/UniformToFill/Center
//   - maui::controls::shapes::point_collection  (Points) — PointCollectionConverter: "x,y x,y …"
//   - maui::controls::shapes::fill_rule         (FillRule) — FillRule names: EvenOdd/Nonzero
//
// SOURCE OF TRUTH: xaml_specs.json "shapes" group + shape.hpp / path.hpp / line.hpp / ellipse.hpp /
// rectangle.hpp / polygon.hpp / polyline.hpp + C# Shape.cs / Path.cs / Line.cs / Polygon.cs …

#include "register_xaml_groups.hpp"

#include <array>
#include <charconv>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_segment.hpp" // point_collection
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/shapes/shape.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/point.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include "register_xaml_helpers.hpp" // register_view_properties<T> (the shared IView/VisualElement surface)

namespace maui::xaml
{
    namespace
    {
        // ---- converter adapter (mirrors xaml_standard_types.cpp's registry_converter) ---------------
        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            };
        }

        // ---- shape-group converters (types not already in register_standard_xaml_converters) -------

        // DoubleCollectionConverter: space- or comma-separated double tokens.
        // C# DoubleCollectionConverter.ConvertFrom: splits on ' ' and ',' and parses each chunk.
        [[nodiscard]] std::vector<double> convert_double_collection(std::string_view text)
        {
            // Replace commas with spaces so a single split pass handles both separators.
            std::string buf(text);
            for (char& ch : buf)
            {
                if (ch == ',')
                {
                    ch = ' ';
                }
            }
            std::vector<double> result;
            std::istringstream stream(buf);
            double value = 0;
            while (stream >> value)
            {
                result.push_back(value);
            }
            if (!stream.eof() && stream.fail())
            {
                throw xaml_convert_error(std::string("Cannot convert \"") + std::string(text) +
                                         "\" to a double collection");
            }
            return result;
        }

        // PenLineCap converter: C# PenLineCap member names (Flat is the C# name for butt).
        // Shape.cs maps PenLineCap onto maui::graphics::line_cap; "Flat" → butt is the only alias.
        [[nodiscard]] maui::graphics::line_cap convert_line_cap(std::string_view text)
        {
            using maui::graphics::line_cap;
            static constexpr std::array<enum_entry<line_cap>, 4> names{{
                {.name = "Flat", .value = line_cap::butt}, // C# PenLineCap.Flat = 0 → the port's butt
                {.name = "Butt", .value = line_cap::butt},
                {.name = "Round", .value = line_cap::round},
                {.name = "Square", .value = line_cap::square},
            }};
            return parse_enum<line_cap>(text, names, "maui::graphics::line_cap");
        }

        // PenLineJoin converter: C# PenLineJoin member names.
        [[nodiscard]] maui::graphics::line_join convert_line_join(std::string_view text)
        {
            using maui::graphics::line_join;
            static constexpr std::array<enum_entry<line_join>, 3> names{{
                {.name = "Miter", .value = line_join::miter},
                {.name = "Round", .value = line_join::round},
                {.name = "Bevel", .value = line_join::bevel},
            }};
            return parse_enum<line_join>(text, names, "maui::graphics::line_join");
        }

        // Shape Stretch → path_aspect converter: C# Stretch names that XAML authors write for the
        // Aspect attribute.  Shape.cs maps C# Stretch → IShapeView.Aspect in its Aspect bindable:
        //   None → none,  Fill → stretch,  Uniform → aspect_fit,  UniformToFill → aspect_fill.
        // path_aspect also has 'center'; no C# Stretch value maps to it, but the spec notes it for
        // completeness — "Center" is not a C# Stretch member, so it is omitted here.
        [[nodiscard]] maui::core::path_aspect convert_path_aspect(std::string_view text)
        {
            using maui::core::path_aspect;
            static constexpr std::array<enum_entry<path_aspect>, 5> names{{
                {.name = "None", .value = path_aspect::none},
                {.name = "Fill", .value = path_aspect::stretch},
                {.name = "Uniform", .value = path_aspect::aspect_fit},
                {.name = "UniformToFill", .value = path_aspect::aspect_fill},
                {.name = "Center", .value = path_aspect::center}, // not a C# Stretch value; accepted defensively
            }};
            return parse_enum<path_aspect>(text, names, "maui::core::path_aspect");
        }

        // PointCollectionConverter: space-separated "x,y" pairs (C# PointCollectionConverter).
        // Accepts "10,100 60,100 35,0" (comma within pair, space between pairs) and the flat form
        // "10 100 60 100 35 0" (all whitespace-separated; pairs are taken two tokens at a time).
        [[nodiscard]] maui::controls::shapes::point_collection convert_point_collection(std::string_view text)
        {
            using maui::controls::shapes::point_collection;
            using maui::graphics::point;

            // Replace commas with spaces and then do a single-pass token scan.
            std::string buf(text);
            for (char& ch : buf)
            {
                if (ch == ',')
                {
                    ch = ' ';
                }
            }
            point_collection result;
            std::istringstream stream(buf);
            double coord = 0;
            while (stream >> coord)
            {
                double coord2 = 0;
                if (!(stream >> coord2))
                {
                    throw xaml_convert_error("PointCollection: odd number of coordinate values");
                }
                result.push_back(point{coord, coord2});
            }
            if (!stream.eof() && stream.fail())
            {
                throw xaml_convert_error(std::string("Cannot convert \"") + std::string(text) +
                                         "\" to a PointCollection");
            }
            return result;
        }

        // FillRule converter: C# FillRule member names.
        [[nodiscard]] maui::controls::shapes::fill_rule convert_fill_rule(std::string_view text)
        {
            using maui::controls::shapes::fill_rule;
            static constexpr std::array<enum_entry<fill_rule>, 2> names{{
                {.name = "EvenOdd", .value = fill_rule::even_odd},
                {.name = "Nonzero", .value = fill_rule::nonzero},
            }};
            return parse_enum<fill_rule>(text, names, "maui::controls::shapes::fill_rule");
        }

        // NOTE: register_view_properties<T> (the shared IView/VisualElement attribute surface — Margin,
        // HorizontalOptions/VerticalOptions/FlowDirection, IsEnabled, Opacity, the transform properties,
        // WidthRequest/HeightRequest family, Clip, BackgroundColor/Background, Style, IsVisible) now comes
        // from the shared register_xaml_helpers.hpp, exactly like every other register_xaml_<group>.cpp
        // TU. This file previously carried its OWN local duplicate that had drifted out of sync (missing
        // Margin/HorizontalOptions/VerticalOptions/FlowDirection/Clip/Background*/Style entirely, so no
        // shape control's XAML surface could set any of them) — deleted in favor of the shared one.

        // Register the shape base-class bindable properties (Fill/Stroke via brush bridge, then
        // the shared shape descriptor slots). Called from each concrete shape block.
        template <class TShape> void register_shape_properties(xaml_property_registry& properties)
        {
            namespace shapes = maui::controls::shapes;

            // Fill and Stroke: XAML authors write a brush string; route through set_fill_brush /
            // set_stroke_brush (shape.hpp X1 — bridges brush→paint, wires BindingContext).
            // register_property<TShape, shared_ptr<brush>> uses the existing brush converter.
            properties.register_property<TShape, std::shared_ptr<maui::controls::brush>>(
                "Fill", [](TShape& shape, const std::shared_ptr<maui::controls::brush>& value) {
                    shape.set_fill_brush(value);
                });
            properties.register_property<TShape, std::shared_ptr<maui::controls::brush>>(
                "Stroke", [](TShape& shape, const std::shared_ptr<maui::controls::brush>& value) {
                    shape.set_stroke_brush(value);
                });

            // Shared shape::*_property() descriptors (one static instance covers the whole family).
            properties.register_bindable_property<TShape>("StrokeThickness",
                                                          shapes::shape::stroke_thickness_property());
            properties.register_bindable_property<TShape>("StrokeDashArray",
                                                          shapes::shape::stroke_dash_array_property());
            properties.register_bindable_property<TShape>("StrokeDashOffset",
                                                          shapes::shape::stroke_dash_offset_property());
            properties.register_bindable_property<TShape>("StrokeLineCap", shapes::shape::stroke_line_cap_property());
            properties.register_bindable_property<TShape>("StrokeLineJoin", shapes::shape::stroke_line_join_property());
            properties.register_bindable_property<TShape>("StrokeMiterLimit",
                                                          shapes::shape::stroke_miter_limit_property());
            properties.register_bindable_property<TShape>("Aspect", shapes::shape::aspect_property());
        }
    } // namespace

    void register_xaml_shapes(xaml_type_registry& types, xaml_property_registry& properties,
                              xaml_converter_registry& converters)
    {
        namespace shapes = maui::controls::shapes;

        // ---- type registration ----------------------------------------------------------------------
        types.register_type<shapes::path>("Path");
        types.register_type<shapes::line>("Line");
        types.register_type<shapes::ellipse>("Ellipse");
        types.register_type<shapes::rectangle>("Rectangle");
        types.register_type<shapes::polygon>("Polygon");
        types.register_type<shapes::polyline>("Polyline");

        // ---- Path (Path.cs) -------------------------------------------------------------------------
        // Data (shared_ptr<geometry>) and RenderTransform (shared_ptr<transform>) are complex object
        // types; no text converter exists or should be fabricated. They require property-element child
        // registration via register_add_child once geometry/transform are registered as XAML types.
        // TODO: register_add_child<shapes::path>("Data", ...) and ("RenderTransform", ...) when
        //       geometry / transform types are added to the XAML type registry.
        register_view_properties<shapes::path>(properties);
        register_shape_properties<shapes::path>(properties);
        // (no control-specific own properties beyond the shape base — path's own bindable surface is
        // data_property / render_transform_property, both deferred above)

        // ---- Line (Line.cs) -------------------------------------------------------------------------
        register_view_properties<shapes::line>(properties);
        register_shape_properties<shapes::line>(properties);
        properties.register_bindable_property<shapes::line>("X1", shapes::line::x1_property());
        properties.register_bindable_property<shapes::line>("Y1", shapes::line::y1_property());
        properties.register_bindable_property<shapes::line>("X2", shapes::line::x2_property());
        properties.register_bindable_property<shapes::line>("Y2", shapes::line::y2_property());

        // ---- Ellipse (Ellipse.cs — no own bindable properties beyond the shape base) ----------------
        register_view_properties<shapes::ellipse>(properties);
        register_shape_properties<shapes::ellipse>(properties);

        // ---- Rectangle (Rectangle.cs) ---------------------------------------------------------------
        register_view_properties<shapes::rectangle>(properties);
        register_shape_properties<shapes::rectangle>(properties);
        properties.register_bindable_property<shapes::rectangle>("RadiusX", shapes::rectangle::radius_x_property());
        properties.register_bindable_property<shapes::rectangle>("RadiusY", shapes::rectangle::radius_y_property());

        // ---- Polygon (Polygon.cs) -------------------------------------------------------------------
        register_view_properties<shapes::polygon>(properties);
        register_shape_properties<shapes::polygon>(properties);
        properties.register_bindable_property<shapes::polygon>("Points", shapes::polygon::points_property());
        properties.register_bindable_property<shapes::polygon>("FillRule", shapes::polygon::fill_rule_property());

        // ---- Polyline (Polyline.cs) -----------------------------------------------------------------
        register_view_properties<shapes::polyline>(properties);
        register_shape_properties<shapes::polyline>(properties);
        properties.register_bindable_property<shapes::polyline>("Points", shapes::polyline::points_property());
        properties.register_bindable_property<shapes::polyline>("FillRule", shapes::polyline::fill_rule_property());

        // ---- converter registrations ----------------------------------------------------------------
        // Types not already in register_standard_xaml_converters (which owns the core 11 controls'
        // converters). Each is registered once here for the whole shapes group.

        // std::vector<double> — StrokeDashArray (DoubleCollectionConverter).
        converters.register_converter<std::vector<double>>(registry_converter(&convert_double_collection));

        // maui::graphics::line_cap — StrokeLineCap (PenLineCap names; "Flat" → butt).
        converters.register_converter<maui::graphics::line_cap>(registry_converter(&convert_line_cap));

        // maui::graphics::line_join — StrokeLineJoin (PenLineJoin names).
        converters.register_converter<maui::graphics::line_join>(registry_converter(&convert_line_join));

        // maui::core::path_aspect — Aspect (C# Stretch names → path_aspect).
        converters.register_converter<maui::core::path_aspect>(registry_converter(&convert_path_aspect));

        // maui::controls::shapes::point_collection — Points (PointCollectionConverter).
        converters.register_converter<maui::controls::shapes::point_collection>(
            registry_converter(&convert_point_collection));

        // maui::controls::shapes::fill_rule — FillRule (FillRule names: EvenOdd/Nonzero).
        converters.register_converter<maui::controls::shapes::fill_rule>(registry_converter(&convert_fill_rule));
    }
} // namespace maui::xaml
