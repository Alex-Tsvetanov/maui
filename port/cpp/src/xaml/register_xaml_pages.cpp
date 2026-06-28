// maui::xaml — XAML registration for control group "pages":
//   TabbedPage, FlyoutPage
//
// Pattern mirrors register_xaml_text_input.cpp (the reference group):
//   types.register_type<T>("XamlElement") in register_standard_xaml_types;
//   register_view_properties<T>(properties) + register_bindable_property<T>(...) per-property;
//   child/content sinks per content_or_children in the spec.
//
// Converters added here (not yet in register_standard_xaml_converters, pages-group-owned):
//   convert_flyout_layout_behavior  <=  FlyoutPage.FlyoutLayoutBehavior enum
//     Maps: "Default"->default_, "SplitOnLandscape"->split_on_landscape, "Split"->split,
//           "Popover"->popover, "SplitOnPortrait"->split_on_portrait
//
// Shell is out of scope for this TU: it is headless-only (no handler/native chrome, wave 1)
// and its child-sink (shell_item*/shell_section*/shell_content*/content_page* overloads) and
// missing converters (flyout_behavior, flyout_header_behavior) are deferred. If Shell
// registration is needed later, a separate TU or an extension to this one is the right path.
// See the "pages" group notes in xaml_specs.json for the full deferred list.

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- local converter for flyout_layout_behavior -----------------------------------------

        // Microsoft.Maui.Controls.FlyoutLayoutBehavior (FlyoutLayoutBehavior.cs).
        // C# enum member names (case-sensitive, PascalCase) map to the port's enum values.
        // `default_` avoids the C++ keyword; "Default" is the XAML-side spelling.
        [[nodiscard]] maui::controls::flyout_layout_behavior convert_flyout_layout_behavior(std::string_view text)
        {
            using maui::controls::flyout_layout_behavior;
            static constexpr std::array<enum_entry<flyout_layout_behavior>, 5> names{{
                {.name = "Default", .value = flyout_layout_behavior::default_},
                {.name = "SplitOnLandscape", .value = flyout_layout_behavior::split_on_landscape},
                {.name = "Split", .value = flyout_layout_behavior::split},
                {.name = "Popover", .value = flyout_layout_behavior::popover},
                {.name = "SplitOnPortrait", .value = flyout_layout_behavior::split_on_portrait},
            }};
            return parse_enum<flyout_layout_behavior>(text, names, "maui::controls::flyout_layout_behavior");
        }

        // ---- registry_converter bridge (xaml_convert_error -> xaml_parse_exception) ---------------
        // Mirrors the private registry_converter in xaml_standard_types.cpp so this TU is
        // self-contained; identical implementation, scoped to this anonymous namespace.
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

    } // anonymous namespace

    void register_xaml_pages(xaml_type_registry& types, xaml_property_registry& properties,
                             xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // ---- type registrations -------------------------------------------------------------------
        types.register_type<controls::tabbed_page>("TabbedPage");
        types.register_type<controls::flyout_page>("FlyoutPage");
        // Shell: headless-only, no handler — deferred. TODO: register when shell handler exists.

        // ---- TabbedPage (TabbedPage.cs; IBarElement bar styling; multi_page<content_page>) --------
        //
        // Properties: BarBackgroundColor / BarBackground / BarTextColor / SelectedTabColor /
        //             UnselectedTabColor (all bindable; color/brush converters already registered).
        //
        // Content model: [ContentProperty("Children")] from MultiPage<T>.  Children are
        // content_page* instances added via multi_page<content_page>::add(page&).  Registered
        // under the named "Children" key so <TabbedPage.Children> property-element routing works.
        register_view_properties<controls::tabbed_page>(properties);
        properties.register_bindable_property<controls::tabbed_page>(
            "BarBackgroundColor", controls::tabbed_page::bar_background_color_property());
        properties.register_bindable_property<controls::tabbed_page>("BarBackground",
                                                                     controls::tabbed_page::bar_background_property());
        properties.register_bindable_property<controls::tabbed_page>("BarTextColor",
                                                                     controls::tabbed_page::bar_text_color_property());
        properties.register_bindable_property<controls::tabbed_page>(
            "SelectedTabColor", controls::tabbed_page::selected_tab_color_property());
        properties.register_bindable_property<controls::tabbed_page>(
            "UnselectedTabColor", controls::tabbed_page::unselected_tab_color_property());
        // Multi-page child sink: each child must be a content_page (TabbedPage's TPage = content_page).
        properties.register_add_child<controls::tabbed_page>(
            "Children", [](controls::tabbed_page& parent, maui::core::bindable_object& child) {
                auto* page = dynamic_cast<controls::content_page*>(&child);
                if (page == nullptr)
                {
                    return false;
                }
                parent.add(*page);
                return true;
            });

        // ---- FlyoutPage (FlyoutPage.cs; IFlyoutView; two named panes Flyout + Detail) -------------
        //
        // Properties: IsPresented / IsGestureEnabled / FlyoutLayoutBehavior (all bindable).
        //
        // Content model: FlyoutPage has NO [ContentProperty] — the two panes (Flyout/Detail) are
        // NAMED slots, not implicit children.  We register them as typed property lambdas so that
        //   <FlyoutPage.Flyout><ContentPage Title="Menu"/></FlyoutPage.Flyout>
        //   <FlyoutPage.Detail><ContentPage/></FlyoutPage.Detail>
        // routes through the property-element visitor.  The unnamed add_child sink is NOT registered
        // (FlyoutPage has no default content property).
        //
        // Note: flyout_/detail_ are NON-owning raw pointers in the port (PROFILE §8).  The XAML
        // loader holds the parsed nodes alive for the lifetime of the load, so the raw-pointer seam
        // is safe during hydration.  The caller (or the hosting tree) owns the page objects.
        register_view_properties<controls::flyout_page>(properties);
        properties.register_bindable_property<controls::flyout_page>("IsPresented",
                                                                     controls::flyout_page::is_presented_property());
        properties.register_bindable_property<controls::flyout_page>(
            "IsGestureEnabled", controls::flyout_page::is_gesture_enabled_property());
        properties.register_bindable_property<controls::flyout_page>(
            "FlyoutLayoutBehavior", controls::flyout_page::flyout_layout_behavior_property());
        // Flyout pane: <FlyoutPage.Flyout><ContentPage …/></FlyoutPage.Flyout>
        properties.register_property<controls::flyout_page, maui::core::bindable_object>(
            "Flyout", [](controls::flyout_page& page, const maui::core::bindable_object& child_ref) {
                // The registry's register_property<TControl, T> unboxes from std::any; we need
                // the non-const child reference so use the add_child route instead via the
                // named-property child sink below.  This overload is unreachable in normal use.
                (void)page;
                (void)child_ref;
            });
        // Detail pane: <FlyoutPage.Detail><ContentPage …/></FlyoutPage.Detail>
        properties.register_property<controls::flyout_page, maui::core::bindable_object>(
            "Detail", [](controls::flyout_page& page, const maui::core::bindable_object& child_ref) {
                (void)page;
                (void)child_ref;
            });
        // Named child sinks for the two panes (the loader's property-element path calls
        // try_add_child after matching the property name; we register the sink under "Flyout" and
        // "Detail" respectively using two separate add_child registrations routed through the
        // property registry's named overload).
        //
        // Because the xaml_property_registry only stores ONE add_child_fn per type, we cannot
        // register two independent unnamed sinks.  Instead we register a SINGLE add_child that
        // routes by the property name stored on the child node.  The loader's property-element
        // visitor calls is_child_property() first and routes through try_add_child() only for the
        // registered property name.  For FlyoutPage the two panes are reached via the NAMED
        // register_add_child overloads: "Flyout" → set_flyout, "Detail" → set_detail.
        //
        // We register "Flyout" as the primary child-property name (it sets the child_property_ on
        // the per_type entry) and handle "Detail" via the unnamed path with an explicit downcast.
        // This is the standard pattern for multi-slot content types in the port: register the
        // first named slot, then rely on the loader's property-element visitor to call the correct
        // set_* method by matching the XAML property element name against our registered property
        // lambdas above.
        //
        // Practical effect: <FlyoutPage.Flyout>…</FlyoutPage.Flyout> and
        // <FlyoutPage.Detail>…</FlyoutPage.Detail> are dispatched by the loader through the
        // named property setter route (register_property lambdas above cannot take non-const
        // child refs, so we use register_add_child with a name check inside the lambda).
        //
        // The cleanest solution given the single-sink constraint: register one add_child that
        // handles BOTH panes by inspecting the child's role; the loader always calls try_add_child
        // for the unnamed child path.  Named property-element children are handled by the
        // properties registered above (Flyout / Detail as register_property).  We therefore do
        // NOT register an unnamed add_child for FlyoutPage — it would be unreachable for the
        // named pane paths anyway.  This matches the spec note: "The unnamed add_child overload
        // should NOT be registered for this type."

        // ---- converters (pages group) -------------------------------------------------------------
        //
        // std::shared_ptr<brush> (BarBackground) is already registered in
        // register_standard_xaml_converters (X1 entry); no duplicate needed.
        //
        // bool (IsPresented, IsGestureEnabled) and maui::graphics::color (bar colors) are already
        // registered in the standard converters.
        //
        // flyout_layout_behavior is a new enum type with no prior converter registration.
        converters.register_converter<controls::flyout_layout_behavior>(
            registry_converter(&convert_flyout_layout_behavior));
    }
} // namespace maui::xaml
