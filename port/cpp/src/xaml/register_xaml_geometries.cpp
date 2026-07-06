// maui::xaml — XAML registration for the GEOMETRY group (View.Clip's element-form value types):
//   RectangleGeometry, EllipseGeometry, GeometryGroup, RoundRectangleGeometry, PathGeometry.
//
// SOURCE OF TRUTH: RectangleGeometry.cs / EllipseGeometry.cs / GeometryGroup.cs /
// RoundRectangleGeometry.cs / PathGeometry.cs (all `: Geometry : BindableObject, IGeometry`).
//
// This closes the `Image.Clip` / `View.Clip` XAML gap (2026-07): the loader previously registered no
// `Clip` property and no geometry element types at all, so `<Image.Clip><RectangleGeometry .../>
// </Image.Clip>` silently dropped the clip layer. `Clip` itself is registered once, generically, in
// register_xaml_helpers.hpp's shared register_view_properties<T> (every view<>-derived control shares
// VisualElement.Clip); THIS file registers the geometry VALUE types `Clip` (and Border.StrokeShape)
// can be assigned from in element form.
//
// Why these types can be register_type'd at all: `geometry` was, until this fix, a plain i_shape-only
// object (no BindableObject base) — a documented PORT COLLAPSE relative to the real C# hierarchy. XAML
// type registration (xaml_type_registry::register_type<T>) requires a bindable_object
// (static_assert'd), so geometry.hpp now derives maui::core::bindable_object too, matching C# exactly.
// Once registered, a created geometry (boxed as shared_ptr<bindable_object> by create_values_visitor)
// reaches a shared_ptr<i_shape>-typed property (View.Clip, Border.StrokeShape) through
// apply_properties_visitor's object-coercion (try_set_created_object<i_shape>, xaml_visitors.cpp) — the
// same route Border.StrokeShape's <Ellipse>/<Rectangle> element form already uses.
//
// Per-type surface:
//   - RectangleGeometry:      Rect (rect)
//   - EllipseGeometry:        Center (point), RadiusX/RadiusY (double)
//   - GeometryGroup:          FillRule (fill_rule) + [ContentProperty("Children")] nested <…Geometry>
//     children — a plain-nested-child sink (register_add_child, unnamed — mirrors
//     register_xaml_brushes.cpp's GradientStop sink). UNLIKE that precedent, each child is a DEEP CLONE
//     (clone_geometry, below) rather than a non-owning aliasing shared_ptr: geometry_group.hpp's
//     ownership doctrine says "a path control / geometry_group owns its geometries via shared_ptr", and
//     a clip geometry routinely outlives the xaml_load_result that created it (Image.Clip keeps only
//     the TOP-LEVEL geometry alive via its own property<shared_ptr<i_shape>> — nothing keeps a
//     non-owning grandchild alive once the loader's xaml_object_graph is destroyed, which segfaulted
//     when tried). The geometry types are small value-like descriptors, so cloning is cheap and exactly
//     reproduces the source's fields.
//   - RoundRectangleGeometry (: GeometryGroup): Rect (rect) + CornerRadius (corner_radius) — the two own
//     attributes (rendering goes through its AppendPath override, not the Children the C# UpdateGeometry
//     rebuild would populate — round_rectangle_geometry.hpp documents the port's equivalent AppendPath-
//     only shortcut), so its Children sink is inherited from GeometryGroup's registration for surface
//     completeness but is not exercised by the shared gallery pages.
//   - PathGeometry:           FillRule (fill_rule) + Figures, TEXT form only ("M8 148 L156 148 L132 12
//     Z" — WPF abbreviated-geometry grammar) via the existing parse_path_figure_collection parser
//     (path_markup_parser.hpp). [ContentProperty("Figures")] element form (nested <PathFigure> markup)
//     is NOT registered — none of the four gallery pages this closes (clip / clip_gallery / clip_views /
//     clip_corner_radius) use it; scoped out, not silently dropped (documented here).
//
// None of these accessors are bindable_property<T> members on the port's geometry classes (plain
// snake_case get/set pairs — see geometry.hpp's PORT COLLAPSE note), so every attribute here is a
// register_property (non-bindable typed lambda), not register_bindable_property.

#include "register_xaml_groups.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/geometry_group.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/shapes/round_rectangle_geometry.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // FillRule converter: C# FillRule member names (shared with register_xaml_shapes.cpp's
        // Polygon/Polyline FillRule — registering it again here is a harmless replace, same behavior;
        // xaml_converter_registry::register_converter is documented "register (or replace)").
        [[nodiscard]] maui::controls::shapes::fill_rule convert_fill_rule(std::string_view text)
        {
            using maui::controls::shapes::fill_rule;
            static constexpr std::array<enum_entry<fill_rule>, 2> names{{
                {.name = "EvenOdd", .value = fill_rule::even_odd},
                {.name = "Nonzero", .value = fill_rule::nonzero},
            }};
            return parse_enum<fill_rule>(text, names, "maui::controls::shapes::fill_rule");
        }

        // Deep-clone a just-created geometry element into a freshly OWNED instance of its concrete type
        // (see the register_xaml_geometries_children_sink comment for why a real owning shared_ptr is
        // needed here instead of the usual non-owning aliasing pattern). Every concrete geometry type
        // register_xaml_geometries registers is covered; an unregistered/unknown geometry type returns
        // nullptr (defensive — none of the five registered types can reach this miss in practice).
        [[nodiscard]] std::shared_ptr<maui::controls::shapes::geometry> clone_geometry(
            const maui::controls::shapes::geometry& source)
        {
            namespace shapes = maui::controls::shapes;
            if (const auto* rect = dynamic_cast<const shapes::rectangle_geometry*>(&source))
            {
                return std::make_shared<shapes::rectangle_geometry>(rect->rect());
            }
            if (const auto* ellipse = dynamic_cast<const shapes::ellipse_geometry*>(&source))
            {
                return std::make_shared<shapes::ellipse_geometry>(ellipse->center(), ellipse->radius_x(),
                                                                  ellipse->radius_y());
            }
            if (const auto* path = dynamic_cast<const shapes::path_geometry*>(&source))
            {
                auto cloned = std::make_shared<shapes::path_geometry>(path->figures(), path->fill_rule());
                return cloned;
            }
            // round_rectangle_geometry : geometry_group, so it must be checked BEFORE the plain
            // geometry_group branch (a dynamic_cast to the base would also match a derived instance).
            if (const auto* round_rect = dynamic_cast<const shapes::round_rectangle_geometry*>(&source))
            {
                auto cloned =
                    std::make_shared<shapes::round_rectangle_geometry>(round_rect->corner_radius(), round_rect->rect());
                cloned->set_fill_rule(round_rect->fill_rule());
                for (const std::shared_ptr<shapes::geometry>& child : round_rect->children())
                {
                    if (child != nullptr)
                    {
                        cloned->children().push_back(clone_geometry(*child));
                    }
                }
                return cloned;
            }
            if (const auto* group = dynamic_cast<const shapes::geometry_group*>(&source))
            {
                auto cloned = std::make_shared<shapes::geometry_group>();
                cloned->set_fill_rule(group->fill_rule());
                for (const std::shared_ptr<shapes::geometry>& child : group->children())
                {
                    if (child != nullptr)
                    {
                        cloned->children().push_back(clone_geometry(*child));
                    }
                }
                return cloned;
            }
            return nullptr;
        }

        // GeometryGroup.Children / [ContentProperty("Children")]: each nested geometry element (any
        // Geometry subtype — RectangleGeometry, EllipseGeometry, GeometryGroup, PathGeometry,
        // RoundRectangleGeometry) is deep-cloned (clone_geometry, above) into the group's children
        // vector, which genuinely OWNS the copy (geometry_group.hpp's ownership doctrine). Templated
        // (not just geometry_group) because the property registry keys the sink by the exact concrete
        // type_tag, not by inheritance — round_rectangle_geometry (: geometry_group) needs its own
        // registration too, same lambda.
        template <class TGroup> void register_geometry_children_sink(xaml_property_registry& properties)
        {
            properties.register_add_child<TGroup>([](TGroup& group, maui::core::bindable_object& child) {
                auto* geom = dynamic_cast<maui::controls::shapes::geometry*>(&child);
                if (geom == nullptr)
                {
                    return false;
                }
                std::shared_ptr<maui::controls::shapes::geometry> owned = clone_geometry(*geom);
                if (owned == nullptr)
                {
                    return false;
                }
                group.children().push_back(std::move(owned));
                return true;
            });
        }
    } // namespace

    void register_xaml_geometries(xaml_type_registry& types, xaml_property_registry& properties,
                                  xaml_converter_registry& converters)
    {
        namespace shapes = maui::controls::shapes;

        // ---- RectangleGeometry (RectangleGeometry.cs) ----
        types.register_type<shapes::rectangle_geometry>("RectangleGeometry");
        properties.register_property<shapes::rectangle_geometry, maui::graphics::rect>(
            "Rect", [](shapes::rectangle_geometry& geom, const maui::graphics::rect& value) { geom.set_rect(value); });

        // ---- EllipseGeometry (EllipseGeometry.cs) ----
        types.register_type<shapes::ellipse_geometry>("EllipseGeometry");
        properties.register_property<shapes::ellipse_geometry, maui::graphics::point>(
            "Center",
            [](shapes::ellipse_geometry& geom, const maui::graphics::point& value) { geom.set_center(value); });
        properties.register_property<shapes::ellipse_geometry, double>(
            "RadiusX", [](shapes::ellipse_geometry& geom, const double& value) { geom.set_radius_x(value); });
        properties.register_property<shapes::ellipse_geometry, double>(
            "RadiusY", [](shapes::ellipse_geometry& geom, const double& value) { geom.set_radius_y(value); });

        // ---- GeometryGroup (GeometryGroup.cs; [ContentProperty("Children")]) ----
        types.register_type<shapes::geometry_group>("GeometryGroup");
        properties.register_property<shapes::geometry_group, shapes::fill_rule>(
            "FillRule",
            [](shapes::geometry_group& group, const shapes::fill_rule& value) { group.set_fill_rule(value); });
        register_geometry_children_sink<shapes::geometry_group>(properties);

        // ---- RoundRectangleGeometry (RoundRectangleGeometry.cs : GeometryGroup) ----
        // Own attributes only (Rect/CornerRadius); FillRule + the Children sink are inherited from the
        // GeometryGroup registration above (same concrete-type-keyed lookup applies per control).
        types.register_type<shapes::round_rectangle_geometry>("RoundRectangleGeometry");
        properties.register_property<shapes::round_rectangle_geometry, maui::graphics::rect>(
            "Rect",
            [](shapes::round_rectangle_geometry& geom, const maui::graphics::rect& value) { geom.set_rect(value); });
        properties.register_property<shapes::round_rectangle_geometry, maui::graphics::corner_radius>(
            "CornerRadius", [](shapes::round_rectangle_geometry& geom, const maui::graphics::corner_radius& value) {
                geom.set_corner_radius(value);
            });
        properties.register_property<shapes::round_rectangle_geometry, shapes::fill_rule>(
            "FillRule",
            [](shapes::round_rectangle_geometry& geom, const shapes::fill_rule& value) { geom.set_fill_rule(value); });
        register_geometry_children_sink<shapes::round_rectangle_geometry>(properties);

        // ---- PathGeometry (PathGeometry.cs) ----
        // Figures: TEXT form only ("M8 148 L156 148 L132 12 Z") via the existing WPF abbreviated-geometry
        // parser (path_markup_parser.hpp); the [ContentProperty("Figures")] nested-<PathFigure> element
        // form is out of scope (header note — none of the four pages this closes use it).
        types.register_type<shapes::path_geometry>("PathGeometry");
        properties.register_property<shapes::path_geometry, std::string>(
            "Figures", [](shapes::path_geometry& geom, const std::string& value) {
                shapes::parse_path_figure_collection(geom.figures(), value);
            });
        properties.register_property<shapes::path_geometry, shapes::fill_rule>(
            "FillRule", [](shapes::path_geometry& geom, const shapes::fill_rule& value) { geom.set_fill_rule(value); });

        // ---- converter registrations ----
        converters.register_converter<shapes::fill_rule>(&convert_fill_rule);
    }
} // namespace maui::xaml
