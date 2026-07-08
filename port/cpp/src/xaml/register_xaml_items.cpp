// maui::xaml — XAML registrations for the templated-collection control group (W4):
//   CollectionView, CarouselView.
//
// These derive items_view (CollectionView via reorderable->groupable->selectable->structured;
// CarouselView via structured), so their handlers are registered in hosting already; W4 only WIRES
// them into the loader. The defining feature of the group is ItemTemplate — a <DataTemplate> body the
// loader inflates per item. That body inflation is handled GENERICALLY: the parser promotes the
// template's single child to _CreateContent, create_values mints a data_template for the <DataTemplate>
// element, and apply_properties_visitor::set_template installs the per-item loader (see
// xaml_template_inflater / xaml_visitors.cpp). So ItemTemplate needs only a typed property route that
// accepts the minted shared_ptr<data_template> and calls set_item_template — there is NO text converter
// for a DataTemplate (it never arrives as a literal).
//
// Property model (find() is an EXACT type-tag lookup with no base walk — register_xaml_helpers.hpp's
// flatten doctrine — so every property is registered under the CONCRETE type):
//   - ItemsSource  (register_bindable_property): set via ItemsSource="{Binding Items}" — a binding, not
//     text, so the binding applier resolves the bindable_name and routes to set_items_source. No text
//     converter (i_item_collection has none; an items source is bound or set in code, never a literal).
//   - ItemTemplate / EmptyViewTemplate (register_property<TControl, shared_ptr<data_template>>): the
//     object route — apply_value_core's typed-std::any path sets it via set_item_template /
//     set_empty_view_template once the <DataTemplate> element value (a shared_ptr<data_template>) reaches
//     the property. No text converter.
//   - SelectionMode (CollectionView only — selectable_items_view; CarouselView derives structured and
//     has no SelectionMode): an enum literal, via the convert_selection_mode converter registered here.
//   - the scroll-bar visibilities (items_view), reusing the converter register_xaml_scrolling_interactive
//     already registered for scroll_bar_visibility.
//
// ItemsLayout (W14): the STRING form ItemsLayout="VerticalGrid,N" / "VerticalList" / "HorizontalGrid,N"
// / "HorizontalList" is supported via convert_items_layout (Microsoft.Maui.Controls.ItemsLayoutType
// Converter) → structured_items_view::set_items_layout. The ELEMENT form <GridItemsLayout Orientation=…
// Span=…> needs the [Parameter("Orientation")] ctor-arg-from-attribute reflection the port lacks
// (Orientation is get-only, ctor-injected), so it stays deferred — the string form is the gallery route.
//
// EmptyView (C# ItemsView.EmptyView, an `object` property): BOTH authored forms are supported through
// ONE raw-setter registration (the registry keys one entry per name, so the setter itself splits on
// the arriving std::any — the same string-vs-view split items_view::set_empty_view's boxed_item
// payload models):
//   - the STRING form (EmptyView="No items…" attribute, or property-element text) arrives as
//     std::string and boxes by value — C# assigns the literal to the object property and the
//     handler renders its ToString;
//   - the ELEMENT form (<CollectionView.EmptyView><Label/></…>) arrives as the created element
//     (boxed shared_ptr<bindable_object> by register_type), passes the type-erased i_view "is a
//     View" check, and boxes with reference semantics — C#'s `EmptyView is View` direct-hosting path.
// The entry's value_type is std::string so apply_value_core's late-text-conversion branch is skipped
// (a literal reaches the setter unconverted; there is deliberately NO boxed_item text converter).
//
// ItemsUpdatingScrollMode is an enum literal on the ItemsView base — registered here (both
// CollectionView and CarouselView) via convert_items_updating_scroll_mode, mirroring SelectionMode.
//
// Deferred (no text converter / binding-or-code only, mirroring register_xaml_scrolling_interactive's
// IndicatorView notes): the <GridItemsLayout> element form (see above), RemainingItemsThreshold (set
// programmatically). CarouselView's Position/CurrentItem (TwoWay, bound or code-driven) are likewise omitted.
//
// See register_xaml_groups.hpp for the function signature declaration; pattern mirrors
// register_xaml_scrolling_interactive.cpp.

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include <any> // EmptyView: the raw-setter registration splits on the arriving std::any
#include <array>
#include <charconv> // W14: std::from_chars (ItemsLayout span parse)
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/controls/items/boxed_item.hpp" // W6/EmptyView: the object stand-in both forms box into
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"          // W14: ItemsLayout="VerticalGrid,N"
#include "maui/controls/items/groupable_items_view.hpp"       // W6: IsGrouped
#include "maui/controls/items/item_sizing_strategy.hpp"       // W6: ItemSizingStrategy enum
#include "maui/controls/items/items_layout.hpp"               // W14: ItemsLayout converter target
#include "maui/controls/items/items_updating_scroll_mode.hpp" // ItemsUpdatingScrollMode enum
#include "maui/controls/items/items_view.hpp"
#include "maui/controls/items/linear_items_layout.hpp" // W14: ItemsLayout="VerticalList"/"HorizontalList"
#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/items/structured_items_view.hpp" // W6: Header/Footer/ItemSizingStrategy
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_view.hpp" // EmptyView: the element form's type-erased "is a View" check
#include "maui/core/type_tag.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp" // enum_entry / parse_enum / xaml_convert_error
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // Bridge a xaml_converters.hpp free function into the registry's error contract (the same
        // registry_converter pattern as register_xaml_scrolling_interactive.cpp).
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

        // convert_selection_mode  <=  Microsoft.Maui.Controls.SelectionMode (Enum.Parse, case-sensitive).
        // C# members: None / Single / Multiple.
        [[nodiscard]] maui::controls::selection_mode convert_selection_mode(std::string_view text)
        {
            using maui::controls::selection_mode;
            static constexpr std::array<enum_entry<selection_mode>, 3> names{{
                {.name = "None", .value = selection_mode::none},
                {.name = "Single", .value = selection_mode::single},
                {.name = "Multiple", .value = selection_mode::multiple},
            }};
            return parse_enum<selection_mode>(text, names, "maui::controls::selection_mode");
        }

        // convert_item_sizing_strategy  <=  Microsoft.Maui.Controls.ItemSizingStrategy (Enum.Parse).
        // C# members: MeasureAllItems / MeasureFirstItem.
        [[nodiscard]] maui::controls::item_sizing_strategy convert_item_sizing_strategy(std::string_view text)
        {
            using maui::controls::item_sizing_strategy;
            static constexpr std::array<enum_entry<item_sizing_strategy>, 2> names{{
                {.name = "MeasureAllItems", .value = item_sizing_strategy::measure_all_items},
                {.name = "MeasureFirstItem", .value = item_sizing_strategy::measure_first_item},
            }};
            return parse_enum<item_sizing_strategy>(text, names, "maui::controls::item_sizing_strategy");
        }

        // convert_items_updating_scroll_mode  <=  Microsoft.Maui.Controls.ItemsUpdatingScrollMode (Enum.Parse).
        // C# members: KeepItemsInView / KeepScrollOffset / KeepLastItemInView.
        [[nodiscard]] maui::controls::items_updating_scroll_mode convert_items_updating_scroll_mode(
            std::string_view text)
        {
            using maui::controls::items_updating_scroll_mode;
            static constexpr std::array<enum_entry<items_updating_scroll_mode>, 3> names{{
                {.name = "KeepItemsInView", .value = items_updating_scroll_mode::keep_items_in_view},
                {.name = "KeepScrollOffset", .value = items_updating_scroll_mode::keep_scroll_offset},
                {.name = "KeepLastItemInView", .value = items_updating_scroll_mode::keep_last_item_in_view},
            }};
            return parse_enum<items_updating_scroll_mode>(text, names, "maui::controls::items_updating_scroll_mode");
        }

        // convert_items_layout (W14) <= Microsoft.Maui.Controls.ItemsLayoutTypeConverter.ConvertFrom:
        //   "VerticalList" / "HorizontalList"          -> a fresh LinearItemsLayout (per-view default)
        //   "VerticalGrid"[,span] / "HorizontalGrid"[,span] -> a GridItemsLayout(span, orientation)
        // This is the idiomatic MAUI XAML form for CollectionView.ItemsLayout (the element form needs the
        // [Parameter("Orientation")] ctor-arg-from-attribute reflection the port lacks). Returns the erased
        // shared_ptr<items_layout> the ItemsLayout property setter takes.
        [[nodiscard]] std::shared_ptr<maui::controls::items_layout> convert_items_layout(std::string_view text)
        {
            namespace controls = maui::controls;
            const std::string_view value = detail::trim(text);
            if (value == "VerticalList")
            {
                return controls::linear_items_layout::create_vertical_default();
            }
            if (value == "HorizontalList")
            {
                return controls::linear_items_layout::create_horizontal_default();
            }

            controls::items_layout_orientation orientation{};
            std::size_t identifier_length = 0;
            if (value.starts_with("VerticalGrid"))
            {
                orientation = controls::items_layout_orientation::vertical;
                identifier_length = std::string_view{"VerticalGrid"}.size();
            }
            else if (value.starts_with("HorizontalGrid"))
            {
                orientation = controls::items_layout_orientation::horizontal;
                identifier_length = std::string_view{"HorizontalGrid"}.size();
            }
            else
            {
                throw xaml_convert_error(std::string{"Cannot convert \""} + std::string{text} +
                                         "\" into maui::controls::items_layout");
            }

            if (value.size() == identifier_length)
            {
                return std::make_shared<controls::grid_items_layout>(orientation);
            }
            if (value.size() > identifier_length + 1 && value[identifier_length] == ',')
            {
                const std::string_view span_text = detail::trim(value.substr(identifier_length + 1));
                int span = 0;
                const auto* last = span_text.data() + span_text.size();
                const auto result = std::from_chars(span_text.data(), last, span);
                if (result.ec == std::errc{} && result.ptr == last && !span_text.empty())
                {
                    return std::make_shared<controls::grid_items_layout>(span, orientation);
                }
            }
            throw xaml_convert_error(std::string{"Cannot convert \""} + std::string{text} +
                                     "\" into maui::controls::items_layout");
        }

        // The shared items_view property surface (ItemsSource / ItemTemplate / EmptyViewTemplate / the
        // scroll-bar visibilities) registered under one CONCRETE type (find() does not walk base types).
        template <class TControl> void register_items_view_surface(xaml_property_registry& properties)
        {
            register_view_properties<TControl>(properties);
            // ItemsSource="{Binding Items}" — a binding; register so the applier resolves the descriptor.
            properties.register_bindable_property<TControl>("ItemsSource",
                                                            maui::controls::items_view::items_source_property());
            // ItemTemplate / EmptyViewTemplate — the object route: the minted shared_ptr<data_template>
            // from the inline <DataTemplate> (or a {StaticResource} key) sets the template.
            properties.register_property<TControl, std::shared_ptr<maui::controls::data_template>>(
                "ItemTemplate", [](TControl& view, const std::shared_ptr<maui::controls::data_template>& tmpl) {
                    view.set_item_template(tmpl);
                });
            properties.register_property<TControl, std::shared_ptr<maui::controls::data_template>>(
                "EmptyViewTemplate", [](TControl& view, const std::shared_ptr<maui::controls::data_template>& tmpl) {
                    view.set_empty_view_template(tmpl);
                });
            // EmptyView — ONE raw registration covering both authored forms (see the header comment):
            // a std::string literal boxes by value; a created element that IS a view (the i_view
            // check — controls::view<> is the CRTP template, so the interface is the type-erased
            // "is a View" test) boxes by reference so the handler hosts the instance directly
            // (C#'s `EmptyView is View` path; boxed_item::as_bindable carries it). Any other payload
            // reports false → the loader's "Cannot assign property" error, matching C# TrySetValue's
            // pre-check.
            properties.register_property<TControl>(
                "EmptyView",
                [](maui::core::bindable_object& target, const std::any& value) -> bool {
                    auto* view = dynamic_cast<TControl*>(&target);
                    if (view == nullptr)
                    {
                        return false;
                    }
                    if (const auto* text = std::any_cast<std::string>(&value))
                    {
                        view->set_empty_view(maui::controls::boxed_item::of(*text));
                        return true;
                    }
                    if (const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(&value))
                    {
                        if (*object != nullptr && dynamic_cast<maui::core::i_view*>(object->get()) != nullptr)
                        {
                            view->set_empty_view(maui::controls::boxed_item::of(*object));
                            return true;
                        }
                    }
                    return false;
                },
                maui::core::type_tag::of<std::string>());
            properties.register_bindable_property<TControl>(
                "HorizontalScrollBarVisibility",
                maui::controls::items_view::horizontal_scroll_bar_visibility_property());
            properties.register_bindable_property<TControl>(
                "VerticalScrollBarVisibility", maui::controls::items_view::vertical_scroll_bar_visibility_property());
            // ItemsUpdatingScrollMode — an enum literal on the ItemsView base (both CollectionView and
            // CarouselView), via the convert_items_updating_scroll_mode converter registered below.
            properties.register_bindable_property<TControl>(
                "ItemsUpdatingScrollMode", maui::controls::items_view::items_updating_scroll_mode_property());

            // W6 — StructuredItemsView surface (both CollectionView and CarouselView derive it):
            //   - Header / Footer: a literal string boxes into the items machinery's reflection-free
            //     object stand-in (boxed_item). A view-typed Header (<CollectionView.Header><Grid/>) is
            //     the object route deferred for now — the string form is what the galleries use.
            //   - HeaderTemplate / FooterTemplate: the same minted-data_template object route as
            //     ItemTemplate (an inline <DataTemplate> or a {StaticResource} key).
            //   - ItemSizingStrategy: an enum literal via the converter registered below.
            properties.register_property<TControl, std::string>("Header", [](TControl& view, const std::string& text) {
                view.set_header(maui::controls::boxed_item::of(text));
            });
            properties.register_property<TControl, std::string>("Footer", [](TControl& view, const std::string& text) {
                view.set_footer(maui::controls::boxed_item::of(text));
            });
            properties.register_property<TControl, std::shared_ptr<maui::controls::data_template>>(
                "HeaderTemplate", [](TControl& view, const std::shared_ptr<maui::controls::data_template>& tmpl) {
                    view.set_header_template(tmpl);
                });
            properties.register_property<TControl, std::shared_ptr<maui::controls::data_template>>(
                "FooterTemplate", [](TControl& view, const std::shared_ptr<maui::controls::data_template>& tmpl) {
                    view.set_footer_template(tmpl);
                });
            properties.register_bindable_property<TControl>(
                "ItemSizingStrategy", maui::controls::structured_items_view::item_sizing_strategy_property());
            // W14 — ItemsLayout="VerticalGrid,N" / "VerticalList" / … : the string form (ItemsLayoutType
            // Converter). structured_items_view::set_items_layout takes the erased shared_ptr<items_layout>;
            // the convert_items_layout converter (registered below) parses the literal. The element form
            // <GridItemsLayout Orientation=… Span=…> needs the [Parameter] ctor-arg reflection the port
            // lacks, so the string form is the supported route.
            properties.register_property<TControl, std::shared_ptr<maui::controls::items_layout>>(
                "ItemsLayout", [](TControl& view, const std::shared_ptr<maui::controls::items_layout>& layout) {
                    view.set_items_layout(layout);
                });
        }
    } // namespace

    void register_xaml_items(xaml_type_registry& types, xaml_property_registry& properties,
                             xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        // ---- CollectionView (CollectionView.cs: the full templated-collection surface, incl.
        //      SelectionMode from selectable_items_view) ----
        types.register_type<controls::collection_view>("CollectionView");
        register_items_view_surface<controls::collection_view>(properties);
        properties.register_bindable_property<controls::collection_view>(
            "SelectionMode", controls::selectable_items_view::selection_mode_property());
        // IsGrouped + the group templates (W6): groupable_items_view — CollectionView derives it,
        // CarouselView does not. GroupHeader/FooterTemplate take the same minted-data_template object
        // route as ItemTemplate / HeaderTemplate.
        properties.register_bindable_property<controls::collection_view>(
            "IsGrouped", controls::groupable_items_view::is_grouped_property());
        properties.register_property<controls::collection_view, std::shared_ptr<controls::data_template>>(
            "GroupHeaderTemplate",
            [](controls::collection_view& view, const std::shared_ptr<controls::data_template>& tmpl) {
                view.set_group_header_template(tmpl);
            });
        properties.register_property<controls::collection_view, std::shared_ptr<controls::data_template>>(
            "GroupFooterTemplate",
            [](controls::collection_view& view, const std::shared_ptr<controls::data_template>& tmpl) {
                view.set_group_footer_template(tmpl);
            });

        // ---- CarouselView (CarouselView.cs: structured_items_view; no SelectionMode/IsGrouped) ----
        types.register_type<controls::carousel_view>("CarouselView");
        register_items_view_surface<controls::carousel_view>(properties);

        // ---- New converters needed by this group (CollectionView.SelectionMode / ItemSizingStrategy /
        //      ItemsLayout) ----
        converters.register_converter<controls::selection_mode>(registry_converter(&convert_selection_mode));
        converters.register_converter<controls::item_sizing_strategy>(
            registry_converter(&convert_item_sizing_strategy));
        converters.register_converter<controls::items_updating_scroll_mode>(
            registry_converter(&convert_items_updating_scroll_mode));
        converters.register_converter<std::shared_ptr<controls::items_layout>>(
            registry_converter(&convert_items_layout));
    }
} // namespace maui::xaml
