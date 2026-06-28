// maui::xaml — XAML registrations for the "scrolling_interactive" control group:
//   ScrollView, RefreshView, SwipeView, IndicatorView.
//
// Content model:
//   - ScrollView, RefreshView, SwipeView: single-child content hosts; Content is wired via
//     register_add_child<T>("Content", ...) casting to maui::core::i_view* and calling set_content.
//   - IndicatorView: leaf control — no child sink or content-property registration needed.
//
// New converters registered here (not in register_standard_xaml_converters):
//   - convert_scroll_orientation  (maui::core::scroll_orientation): Vertical/Horizontal/Both/Neither
//   - convert_scroll_bar_visibility (maui::core::scroll_bar_visibility): Default/Always/Never
//   - convert_swipe_transition_mode (maui::core::swipe_transition_mode): Reveal/Drag
//   - convert_indicator_shape     (maui::controls::indicator_shape): Circle/Square
//
// Deferred (no bindable_property accessor, or no text converter and binding-only):
//   - SwipeView LeftItems/RightItems/TopItems/BottomItems: owned swipe_items collections with no
//     text converter — deferred until property-element + typed child-sink support lands.
//   - IndicatorView ItemsSource / Count: std::shared_ptr<i_item_collection> has no text converter;
//     Count is driven programmatically from ItemsSource; both omitted here.
//
// Pattern mirrors register_xaml_range_progress.cpp and uses the register_view_properties<T> helper
// from register_xaml_helpers.hpp to flatten the shared IView/VisualElement attribute surface.
//
// See register_xaml_groups.hpp for the function signature declaration.

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include <array>
#include <span>

#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/indicator_view.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp" // also defines xaml_convert_error
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // Bridge a xaml_converters.hpp free function into the registry's error contract
        // (same registry_converter pattern as xaml_standard_types.cpp).
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

        // convert_scroll_orientation  <=  Microsoft.Maui.ScrollOrientation (Enum.Parse, case-sensitive).
        // C# members: Vertical / Horizontal / Both / Neither.
        [[nodiscard]] maui::core::scroll_orientation convert_scroll_orientation(std::string_view text)
        {
            using maui::core::scroll_orientation;
            static constexpr std::array<enum_entry<scroll_orientation>, 4> names{{
                {.name = "Vertical", .value = scroll_orientation::vertical},
                {.name = "Horizontal", .value = scroll_orientation::horizontal},
                {.name = "Both", .value = scroll_orientation::both},
                {.name = "Neither", .value = scroll_orientation::neither},
            }};
            return parse_enum<scroll_orientation>(text, names, "maui::core::scroll_orientation");
        }

        // convert_scroll_bar_visibility  <=  Microsoft.Maui.ScrollBarVisibility (Enum.Parse,
        // case-sensitive). C# members: Default / Always / Never. Note: port spells the first
        // enumerator `default_` (reserved keyword avoidance); the name table maps the C# "Default"
        // string to that enumerator.
        [[nodiscard]] maui::core::scroll_bar_visibility convert_scroll_bar_visibility(std::string_view text)
        {
            using maui::core::scroll_bar_visibility;
            static constexpr std::array<enum_entry<scroll_bar_visibility>, 3> names{{
                {.name = "Default", .value = scroll_bar_visibility::default_},
                {.name = "Always", .value = scroll_bar_visibility::always},
                {.name = "Never", .value = scroll_bar_visibility::never},
            }};
            return parse_enum<scroll_bar_visibility>(text, names, "maui::core::scroll_bar_visibility");
        }

        // convert_swipe_transition_mode  <=  Microsoft.Maui.Controls.SwipeTransitionMode (Enum.Parse,
        // case-sensitive). C# members: Reveal / Drag.
        [[nodiscard]] maui::core::swipe_transition_mode convert_swipe_transition_mode(std::string_view text)
        {
            using maui::core::swipe_transition_mode;
            static constexpr std::array<enum_entry<swipe_transition_mode>, 2> names{{
                {.name = "Reveal", .value = swipe_transition_mode::reveal},
                {.name = "Drag", .value = swipe_transition_mode::drag},
            }};
            return parse_enum<swipe_transition_mode>(text, names, "maui::core::swipe_transition_mode");
        }

        // convert_indicator_shape  <=  Microsoft.Maui.Controls.IndicatorShape (Enum.Parse,
        // case-sensitive). C# members: Circle / Square.
        [[nodiscard]] maui::controls::indicator_shape convert_indicator_shape(std::string_view text)
        {
            using maui::controls::indicator_shape;
            static constexpr std::array<enum_entry<indicator_shape>, 2> names{{
                {.name = "Circle", .value = indicator_shape::circle},
                {.name = "Square", .value = indicator_shape::square},
            }};
            return parse_enum<indicator_shape>(text, names, "maui::controls::indicator_shape");
        }
    } // namespace

    void register_xaml_scrolling_interactive(xaml_type_registry& types, xaml_property_registry& properties,
                                             xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // ---- ScrollView (ScrollView.cs: single-child content host + Padding + Orientation +
        //      horizontal/vertical scroll-bar visibility + SafeAreaEdges) ----
        types.register_type<controls::scroll_view>("ScrollView");
        register_view_properties<controls::scroll_view>(properties);
        properties.register_bindable_property<controls::scroll_view>("Padding",
                                                                     controls::scroll_view::padding_property());
        properties.register_bindable_property<controls::scroll_view>("Orientation",
                                                                     controls::scroll_view::orientation_property());
        properties.register_bindable_property<controls::scroll_view>(
            "HorizontalScrollBarVisibility", controls::scroll_view::horizontal_scroll_bar_visibility_property());
        properties.register_bindable_property<controls::scroll_view>(
            "VerticalScrollBarVisibility", controls::scroll_view::vertical_scroll_bar_visibility_property());
        properties.register_bindable_property<controls::scroll_view>("SafeAreaEdges",
                                                                     controls::scroll_view::safe_area_edges_property());
        // C# ScrollView [ContentProperty("Content")]: single-child content host.
        properties.register_add_child<controls::scroll_view>(
            "Content", [](controls::scroll_view& scroll, maui::core::bindable_object& child) {
                auto* view = dynamic_cast<maui::core::i_view*>(&child);
                if (view == nullptr)
                {
                    return false;
                }
                scroll.set_content(*view);
                return true;
            });

        // ---- RefreshView (RefreshView.cs: pull-to-refresh content host; IsRefreshing /
        //      IsRefreshEnabled + RefreshColor + Padding) ----
        types.register_type<controls::refresh_view>("RefreshView");
        register_view_properties<controls::refresh_view>(properties);
        properties.register_bindable_property<controls::refresh_view>("IsRefreshing",
                                                                      controls::refresh_view::is_refreshing_property());
        // C# RefreshView.IsEnabled in XAML routes to the IsRefreshEnabled bindable on the port
        // (the view-level IsEnabled is the VisualElement one registered by register_view_properties).
        // The spec maps the XAML "IsEnabled" attribute on RefreshView to is_refresh_enabled_property().
        properties.register_bindable_property<controls::refresh_view>(
            "IsEnabled", controls::refresh_view::is_refresh_enabled_property());
        properties.register_bindable_property<controls::refresh_view>("RefreshColor",
                                                                      controls::refresh_view::refresh_color_property());
        properties.register_bindable_property<controls::refresh_view>("Padding",
                                                                      controls::refresh_view::padding_property());
        // C# RefreshView [ContentProperty("Content")] (inherited from ContentView): single-child.
        properties.register_add_child<controls::refresh_view>(
            "Content", [](controls::refresh_view& refresh, maui::core::bindable_object& child) {
                auto* view = dynamic_cast<maui::core::i_view*>(&child);
                if (view == nullptr)
                {
                    return false;
                }
                refresh.set_content(*view);
                return true;
            });

        // ---- SwipeView (SwipeView.cs: swipeable content host; Threshold + Padding +
        //      SwipeTransitionMode [non-bindable]) ----
        types.register_type<controls::swipe_view>("SwipeView");
        register_view_properties<controls::swipe_view>(properties);
        properties.register_bindable_property<controls::swipe_view>("Threshold",
                                                                    controls::swipe_view::threshold_property());
        properties.register_bindable_property<controls::swipe_view>("Padding",
                                                                    controls::swipe_view::padding_property());
        // SwipeTransitionMode is a plain (non-bindable) member (transition_mode_ has no
        // bindable_property<> descriptor). Route via register_property with a typed lambda.
        properties.register_property<controls::swipe_view, maui::core::swipe_transition_mode>(
            "SwipeTransitionMode", [](controls::swipe_view& sv, const maui::core::swipe_transition_mode& value) {
                sv.set_transition_mode(value);
            });
        // C# SwipeView [ContentProperty("Content")] (inherited from ContentView): single-child.
        // LeftItems/RightItems/TopItems/BottomItems are deferred (no text converter for the owned
        // swipe_items collections — property-element support is a future loader extension).
        properties.register_add_child<controls::swipe_view>(
            "Content", [](controls::swipe_view& swipe, maui::core::bindable_object& child) {
                auto* view = dynamic_cast<maui::core::i_view*>(&child);
                if (view == nullptr)
                {
                    return false;
                }
                swipe.set_content(*view);
                return true;
            });

        // ---- IndicatorView (IndicatorView.cs: indicator dots; leaf control — no child sink) ----
        // ItemsSource (std::shared_ptr<i_item_collection>) and Count (read-only, driven by
        // ItemsSource) are intentionally omitted: no text converter exists and they are set
        // programmatically or via binding, not from XAML text literals.
        types.register_type<controls::indicator_view>("IndicatorView");
        register_view_properties<controls::indicator_view>(properties);
        properties.register_bindable_property<controls::indicator_view>(
            "IndicatorsShape", controls::indicator_view::indicators_shape_property());
        properties.register_bindable_property<controls::indicator_view>("Position",
                                                                        controls::indicator_view::position_property());
        properties.register_bindable_property<controls::indicator_view>(
            "MaximumVisible", controls::indicator_view::maximum_visible_property());
        properties.register_bindable_property<controls::indicator_view>(
            "HideSingle", controls::indicator_view::hide_single_property());
        properties.register_bindable_property<controls::indicator_view>(
            "IndicatorColor", controls::indicator_view::indicator_color_property());
        properties.register_bindable_property<controls::indicator_view>(
            "SelectedIndicatorColor", controls::indicator_view::selected_indicator_color_property());
        properties.register_bindable_property<controls::indicator_view>(
            "IndicatorSize", controls::indicator_view::indicator_size_property());

        // ---- New converters needed by this group ----
        converters.register_converter<maui::core::scroll_orientation>(registry_converter(&convert_scroll_orientation));
        converters.register_converter<maui::core::scroll_bar_visibility>(
            registry_converter(&convert_scroll_bar_visibility));
        converters.register_converter<maui::core::swipe_transition_mode>(
            registry_converter(&convert_swipe_transition_mode));
        converters.register_converter<maui::controls::indicator_shape>(registry_converter(&convert_indicator_shape));
    }
} // namespace maui::xaml
