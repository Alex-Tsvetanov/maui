// maui::xaml — XAML registration for the "containers_content" group:
//   ContentView, Border, Frame, BoxView
//
// SOURCE OF TRUTH: xaml_specs.json group "containers_content" (generated from header audit).
// PATTERN: mirrors register_xaml_text_input.cpp — same includes style, same namespace, same call
// sequence: types.register_type / register_view_properties / register_bindable_property / content
// child sink (register_add_child).
//
// Per-property converter gaps (documented deferrals — registered properties still compile, but the
// converter registry has no entry for these types so XAML text attributes will fail at runtime
// rather than at registration time):
//   Border.Stroke       (std::shared_ptr<maui::graphics::paint>)   — convert_paint (W6: DONE below)
//   Border.StrokeShape  (std::shared_ptr<maui::graphics::i_shape>)  — needs convert_stroke_shape
//   Border.StrokeDashArray (std::vector<double>)                    — needs convert_double_list
//   Border.StrokeLineCap   (maui::graphics::line_cap)               — needs convert_line_cap
//   Border.StrokeLineJoin  (maui::graphics::line_join)              — needs convert_line_join
// All other property types (double, bool, float, maui::graphics::color, maui::core::thickness,
// maui::core::safe_area_edges, maui::graphics::corner_radius) already have registered converters.

#include "register_xaml_groups.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/border.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/templates/content_presenter.hpp" // W16: <ContentPresenter> in a ControlTemplate body
#include "maui/controls/templates/control_template.hpp"  // W16: ContentView.ControlTemplate
#include "maui/controls/templates/templated_view.hpp"    // W6: ContentView.Padding (inherited descriptor)
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/paint.hpp"       // Border.Stroke value type (W6 convert_paint)
#include "maui/graphics/solid_paint.hpp" // a color literal -> solid_paint (W6 convert_paint)
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"      // convert_color + xaml_convert_error (W6 convert_paint)
#include "maui/xaml/xaml_parse_exception.hpp" // the loader's single error channel (W6 convert_paint)
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"
#include "register_xaml_helpers.hpp"

namespace maui::xaml
{
    namespace
    {
        // convert_paint (W6) — a paint-typed attribute (Border.Stroke, and any Shape Stroke/Fill) set from
        // a COLOR literal becomes a solid_paint, mirroring C#'s BrushTypeConverter/Color->SolidColorBrush
        // coercion for the IStroke surface. Element-form gradients (<Border.Stroke><LinearGradientBrush/>)
        // are the object route and do not pass through here. convert_color throws xaml_convert_error →
        // translate to the loader's single xaml_parse_exception channel.
        [[nodiscard]] std::shared_ptr<maui::graphics::paint> convert_paint(const std::string& text)
        {
            try
            {
                return std::make_shared<maui::graphics::solid_paint>(convert_color(text));
            }
            catch (const xaml_convert_error& error)
            {
                throw xaml_parse_exception(error.what());
            }
        }
    } // namespace

    void register_xaml_containers_content(xaml_type_registry& types, xaml_property_registry& properties,
                                          xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // The general paint converter (Border.Stroke + shape Stroke/Fill); registered once, keyed by type.
        converters.register_converter<std::shared_ptr<maui::graphics::paint>>(&convert_paint);

        // ---- ContentView (ContentView.cs [ContentProperty("Content")]) ----
        // C# ContentView exposes only SafeAreaEdges as its own bindable property; Content is routed
        // through set_content(shared_ptr<element>) — a shared_ptr seam, unlike the i_view* seam on
        // border/frame. The child lambda uses a NON-owning aliasing shared_ptr so the XAML graph
        // retains ownership; the content_view co-owns its content via the assignment in set_content.
        types.register_type<controls::content_view>("ContentView");
        register_view_properties<controls::content_view>(properties);
        properties.register_bindable_property<controls::content_view>(
            "SafeAreaEdges", controls::content_view::safe_area_edges_property());
        // Padding (W6): ContentView inherits it from templated_view (C# Layout) — register the inherited
        // descriptor so <ContentView Padding="…"> works, mirroring Border/Frame.
        properties.register_bindable_property<controls::content_view>("Padding",
                                                                      controls::templated_view::padding_property());
        // W16 — ControlTemplate: the minted <ControlTemplate> (shared_ptr<control_template>) routes to
        // set_control_template, which runs template_utilities::on_control_template_changed (stamps the
        // template root + wires its ContentPresenter to pull the developer Content). Element form:
        // <ContentView.ControlTemplate><ControlTemplate>…</ControlTemplate></ContentView.ControlTemplate>.
        properties.register_property<controls::content_view, std::shared_ptr<controls::control_template>>(
            "ControlTemplate", [](controls::content_view& cv, const std::shared_ptr<controls::control_template>& tmpl) {
                cv.set_control_template(tmpl);
            });
        properties.register_add_child<controls::content_view>(
            "Content", [](controls::content_view& cv, maui::core::bindable_object& child) {
                auto* elem = dynamic_cast<controls::element*>(&child);
                if (elem == nullptr)
                {
                    return false;
                }
                // Non-owning aliasing shared_ptr: content_view::set_content takes shared_ptr<element>;
                // the XAML graph owns the object, so we pass a handle that will not double-free it.
                cv.set_content(std::shared_ptr<controls::element>(std::shared_ptr<void>{}, elem));
                return true;
            });

        // ---- ContentPresenter (W16) — the seam inside a ControlTemplate body that hosts the templated
        //      control's developer Content. It is a real view (default-constructible); its ctor installs a
        //      TemplatedParent.Content template-binding, so it pulls the content automatically once the
        //      template is stamped — no markup content sink needed. Only Padding is settable from markup. ----
        types.register_type<controls::content_presenter>("ContentPresenter");
        register_view_properties<controls::content_presenter>(properties);
        properties.register_bindable_property<controls::content_presenter>(
            "Padding", controls::content_presenter::padding_property());

        // ---- Border (Border.cs) ----
        // Ten bindable properties on the stroke surface. Stroke (shared_ptr<paint>), StrokeShape
        // (shared_ptr<i_shape>), StrokeDashArray (vector<double>), StrokeLineCap (line_cap), and
        // StrokeLineJoin (line_join) each need a converter not yet in the registry — registered here
        // for apply_setter completeness; text-attribute use defers until the converters are added.
        types.register_type<controls::border>("Border");
        register_view_properties<controls::border>(properties);
        properties.register_bindable_property<controls::border>("Padding", controls::border::padding_property());
        properties.register_bindable_property<controls::border>("StrokeThickness",
                                                                controls::border::stroke_thickness_property());
        properties.register_bindable_property<controls::border>("StrokeDashOffset",
                                                                controls::border::stroke_dash_offset_property());
        properties.register_bindable_property<controls::border>("StrokeMiterLimit",
                                                                controls::border::stroke_miter_limit_property());
        properties.register_bindable_property<controls::border>("SafeAreaEdges",
                                                                controls::border::safe_area_edges_property());
        // Stroke now settable from a color literal via convert_paint (above); StrokeShape/DashArray/
        // LineCap/LineJoin remain converter-deferred (see header comment).
        properties.register_bindable_property<controls::border>("Stroke", controls::border::stroke_property());
        properties.register_bindable_property<controls::border>("StrokeShape",
                                                                controls::border::stroke_shape_property());
        properties.register_bindable_property<controls::border>("StrokeDashArray",
                                                                controls::border::stroke_dash_array_property());
        properties.register_bindable_property<controls::border>("StrokeLineCap",
                                                                controls::border::stroke_line_cap_property());
        properties.register_bindable_property<controls::border>("StrokeLineJoin",
                                                                controls::border::stroke_line_join_property());
        // C# Border.Content [ContentProperty] — non-owning i_view* seam (border.set_content(i_view*)).
        properties.register_add_child<controls::border>("Content",
                                                        [](controls::border& b, maui::core::bindable_object& child) {
                                                            auto* view = dynamic_cast<maui::core::i_view*>(&child);
                                                            if (view == nullptr)
                                                            {
                                                                return false;
                                                            }
                                                            b.set_content(view);
                                                            return true;
                                                        });

        // ---- Frame (Frame.cs; derives border; four own bindable properties) ----
        // Frame.padding_property() shadows border's with a default of 20 (Frame.PaddingDefaultValueCreator).
        // Frame.Content child sink is identical to Border's — same set_content(i_view*) seam, inherited.
        types.register_type<controls::frame>("Frame");
        register_view_properties<controls::frame>(properties);
        properties.register_bindable_property<controls::frame>("Padding", controls::frame::padding_property());
        properties.register_bindable_property<controls::frame>("BorderColor", controls::frame::border_color_property());
        properties.register_bindable_property<controls::frame>("CornerRadius",
                                                               controls::frame::corner_radius_property());
        properties.register_bindable_property<controls::frame>("HasShadow", controls::frame::has_shadow_property());
        properties.register_add_child<controls::frame>("Content",
                                                       [](controls::frame& f, maui::core::bindable_object& child) {
                                                           auto* view = dynamic_cast<maui::core::i_view*>(&child);
                                                           if (view == nullptr)
                                                           {
                                                               return false;
                                                           }
                                                           f.set_content(view);
                                                           return true;
                                                       });

        // ---- BoxView (BoxView.cs; leaf IShapeView; no child content property) ----
        // Color (maui::graphics::color) and CornerRadius (maui::graphics::corner_radius) both have
        // registered converters (convert_color, convert_corner_radius). No child/content sink needed.
        types.register_type<controls::box_view>("BoxView");
        register_view_properties<controls::box_view>(properties);
        properties.register_bindable_property<controls::box_view>("Color", controls::box_view::color_property());
        properties.register_bindable_property<controls::box_view>("CornerRadius",
                                                                  controls::box_view::corner_radius_property());
    }
} // namespace maui::xaml
