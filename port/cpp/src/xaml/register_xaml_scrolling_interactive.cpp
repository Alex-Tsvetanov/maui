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
//   - convert_swipe_mode          (maui::core::swipe_mode): Reveal/Execute (SwipeItems.Mode)
//   - convert_swipe_behavior_on_invoked (maui::core::swipe_behavior_on_invoked): Auto/Close/RemainOpen
//   - convert_indicator_shape     (maui::controls::indicator_shape): Circle/Square
//
// SwipeView items surface (SwipeItem / SwipeItemView / SwipeItems) is registered here too. The four
// directional collections (SwipeView.LeftItems/RightItems/TopItems/BottomItems) CANNOT use a named
// register_add_child sink — SwipeView already spends its single child-sink slot on "Content", and the
// per-type registry holds ONE add_child + ONE child_property (xaml_property_registry.hpp). So the four
// property-elements are routed by a Grid/Picker-style special-case in xaml_visitors.cpp
// (try_add_swipe_view_items) into the pre-existing OWNED default collections via *_items_collection().
//
// Deferred (no bindable_property accessor, or no text converter and binding-only):
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
#include "maui/controls/menu_item.hpp" // SwipeItem inherits Text/IsEnabled/IconImageSource from MenuItem
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_swipe_item.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/swipe_behavior_on_invoked.hpp"
#include "maui/core/swipe_mode.hpp"
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

        // convert_swipe_mode  <=  Microsoft.Maui.SwipeMode (Enum.Parse, case-sensitive).
        // C# members: Reveal / Execute (SwipeMode.cs). Backs SwipeItems.Mode.
        [[nodiscard]] maui::core::swipe_mode convert_swipe_mode(std::string_view text)
        {
            using maui::core::swipe_mode;
            static constexpr std::array<enum_entry<swipe_mode>, 2> names{{
                {.name = "Reveal", .value = swipe_mode::reveal},
                {.name = "Execute", .value = swipe_mode::execute},
            }};
            return parse_enum<swipe_mode>(text, names, "maui::core::swipe_mode");
        }

        // convert_swipe_behavior_on_invoked  <=  Microsoft.Maui.SwipeBehaviorOnInvoked (Enum.Parse,
        // case-sensitive). C# members: Auto / Close / RemainOpen (SwipeBehaviorOnInvoked.cs). Note the
        // port spells the first enumerator `automatic` (avoiding the C++ reserved-word feel of `auto`);
        // the name table maps the C# "Auto" string to that enumerator. Backs SwipeItems.SwipeBehaviorOnInvoked.
        [[nodiscard]] maui::core::swipe_behavior_on_invoked convert_swipe_behavior_on_invoked(std::string_view text)
        {
            using maui::core::swipe_behavior_on_invoked;
            static constexpr std::array<enum_entry<swipe_behavior_on_invoked>, 3> names{{
                {.name = "Auto", .value = swipe_behavior_on_invoked::automatic},
                {.name = "Close", .value = swipe_behavior_on_invoked::close},
                {.name = "RemainOpen", .value = swipe_behavior_on_invoked::remain_open},
            }};
            return parse_enum<swipe_behavior_on_invoked>(text, names, "maui::core::swipe_behavior_on_invoked");
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
        // LeftItems/RightItems/TopItems/BottomItems are NOT registered as named child sinks (SwipeView's
        // single child-sink slot is spent here on "Content"); the four directional property-elements are
        // routed by try_add_swipe_view_items in xaml_visitors.cpp into swipe_view's OWNED default
        // collections. See this file's header note.
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

        // ---- SwipeItem (SwipeItem.cs: a MenuItem shown in a swipe. Text/IsEnabled/IconImageSource come
        //      from the menu_item base; BackgroundColor + IsVisible are its own bindables. Not a view<>,
        //      so NO register_view_properties.) ----
        types.register_type<controls::swipe_item>("SwipeItem");
        properties.register_bindable_property<controls::swipe_item>("Text", controls::menu_item::text_property());
        properties.register_bindable_property<controls::swipe_item>("IsEnabled",
                                                                    controls::menu_item::is_enabled_property());
        properties.register_bindable_property<controls::swipe_item>("IconImageSource",
                                                                    controls::menu_item::icon_image_source_property());
        properties.register_bindable_property<controls::swipe_item>("BackgroundColor",
                                                                    controls::swipe_item::background_color_property());
        properties.register_bindable_property<controls::swipe_item>("IsVisible",
                                                                    controls::swipe_item::is_visible_property());

        // ---- SwipeItemView (SwipeItemView.cs: a ContentView-shaped swipe item hosting one Content.
        //      IS a view<>, so it gets the shared IView/VisualElement surface + Padding + a Content sink,
        //      exactly like content_page.) ----
        types.register_type<controls::swipe_item_view>("SwipeItemView");
        register_view_properties<controls::swipe_item_view>(properties);
        properties.register_bindable_property<controls::swipe_item_view>("Padding",
                                                                         controls::swipe_item_view::padding_property());
        // C# SwipeItemView [ContentProperty("Content")] (inherited from ContentView): single-child.
        properties.register_add_child<controls::swipe_item_view>(
            "Content", [](controls::swipe_item_view& item, maui::core::bindable_object& child) {
                auto* view = dynamic_cast<maui::core::i_view*>(&child);
                if (view == nullptr)
                {
                    return false;
                }
                item.set_content(*view);
                return true;
            });

        // ---- SwipeItems (SwipeItems.cs: the collection wrapping the items for one direction. Mode +
        //      SwipeBehaviorOnInvoked bindables + an UNNAMED collection child sink so <SwipeItems>'
        //      <SwipeItem>/<SwipeItemView> children add straight to it (the FormattedString.Spans shape).
        //      Not a view<>, so NO register_view_properties.) ----
        types.register_type<controls::swipe_items>("SwipeItems");
        properties.register_bindable_property<controls::swipe_items>("Mode", controls::swipe_items::mode_property());
        properties.register_bindable_property<controls::swipe_items>(
            "SwipeBehaviorOnInvoked", controls::swipe_items::behavior_on_invoked_property());
        // C# SwipeItems [ContentProperty(...)] over IList<ISwipeItem>: the bare-element list sink. Both
        // swipe_item and swipe_item_view implement i_swipe_item; the collection stores the NON-owning
        // i_swipe_item face (the graph owns each created item), mirroring how swipe_items itself keeps
        // NON-owning i_swipe_item* pointers.
        properties.register_add_child<controls::swipe_items>(
            [](controls::swipe_items& items, maui::core::bindable_object& child) {
                auto* item = dynamic_cast<maui::core::i_swipe_item*>(&child);
                if (item == nullptr)
                {
                    return false;
                }
                items.add(*item);
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
        converters.register_converter<maui::core::swipe_mode>(registry_converter(&convert_swipe_mode));
        converters.register_converter<maui::core::swipe_behavior_on_invoked>(
            registry_converter(&convert_swipe_behavior_on_invoked));
        converters.register_converter<maui::controls::indicator_shape>(registry_converter(&convert_indicator_shape));
    }
} // namespace maui::xaml
