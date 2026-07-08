#pragma once
// maui::samples::nested_collection_page — ports NestedGalleries/NestedCollectionViewGallery.xaml
//   (+ NestedCollectionViewGallery.xaml.cs) of the C# CollectionView gallery.
//
// The C# page ("It's CollectionViews all the way down."): a StackLayout with a Label
// (Text="It's CollectionViews all the way down.") above an OUTER CollectionView (ItemsLayout
// VerticalList) whose ItemTemplate is a Grid (two rows: an italic red Label bound to the group
// {Binding Title}, then a Grid.Row=1 INNER CollectionView — ItemsLayout HorizontalList,
// HeightRequest=100, scroll bars Never — bound to that same outer item's {Binding Items}). The inner
// CollectionView's own ItemTemplate is a Grid (an Image bound to {Binding Image} over a blue 10pt
// Label bound to {Binding Caption}). So: an outer vertical list of "sources", each rendering its
// Title plus a horizontal inner list of its image/caption items — a CollectionView whose cells host
// CollectionViews ("Yo, dawg, we heard you like CollectionViews…").
//
// xaml.cs model: NestedCollectionViewModel.Items = 20 NestedItemSource("Source 0".."Source 19"); each
// NestedItemSource has a Title + an Items list of 6..15 CollectionViewGalleryTestItem (Image+Caption),
// drawn from a DemoFilteredItemSource. BindingContext = the view model.
//
// This headless port owns its whole tree (the items_page pattern) and reproduces the NESTING directly:
//   - nested_source is the reflection-free NestedItemSource KEY half: Title (outer cell's red Label)
//     plus the inner item collection it exposes as {Binding Items} (a shared i_item_collection the
//     inner CV binds its ItemsSource to);
//   - gallery_item is the reflection-free CollectionViewGalleryTestItem subset the inner cell binds:
//     Caption (the blue Label). C#'s Image source has no headless asset pipeline, so the Image is the
//     documented reduction (the sibling CollectionView pages' Image→omitted convention); the inner cell
//     is the bound caption Label;
//   - the OUTER item template is data_template::of<collection_view>() — the cell content IS the inner
//     collection_view (confirmed: of<TControl> activates any default-constructible bindable_object, and
//     collection_view is one). Its inner ItemTemplate is STAGED on the outer template via set_value
//     (item_template_property is bindable), and its inner ItemsSource is BOUND off each outer item via
//     set_binding (items_source_property is bindable<shared_ptr<i_item_collection>>) — so each realized
//     inner CV gets the matching outer source's Items as its source. HorizontalList layout + the
//     HeightRequest=100 + scroll-bars-Never chrome are staged on the inner CV too.
//
// note: the C# OUTER cell is a two-row Grid (an italic red Title Label ABOVE the inner CV). The port's
//       templated cells render a SINGLE root control (data_template::of<TControl>), so the cell root is
//       the inner collection_view, and the outer item's Title surfaces as that inner CV's HEADER — but
//       rendered through a red-italic header TEMPLATE bound to the source Title (matching the XAML
//       TextColor=Red / FontAttributes=Italic), not a plain boxed string, so the title color is faithful
//       (documented reduction in STRUCTURE only — the title is the inner list's header rather than a
//       separate Grid row; the bound data — Title + the inner image/caption captions — and its color are
//       identical). The headless virtualization sim realizes the OUTER cells (each an inner
//       collection_view bound to its source); headless has no live native pump to recursively realize
//       each inner CV's own cells, so there the nesting is wired + bound but the inner cells' geometry is
//       not arranged. On the iOS backend the inner UICollectionViews DO recursively realize + self-size
//       their own cells (the captions render), so the full nesting is visible; the inner CV's HORIZONTAL
//       orientation is now staged through the outer template via a setup action (add_setup), so the inner
//       cells flow in a horizontal row as in MAUI.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_view.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class nested_collection_page
    {
    public:
        // The reflection-free CollectionViewGalleryTestItem subset the INNER cell binds: Caption (the
        // blue 10pt Label). The C# Image source is the documented reduction (no headless asset pipeline).
        struct gallery_item
        {
            std::string caption;
            friend bool operator==(const gallery_item&, const gallery_item&) = default;
        };

        // The reflection-free NestedItemSource KEY half (the OUTER cell's binding context): Title (the
        // red italic Label / the inner CV's header) plus the inner item collection it exposes as
        // {Binding Items}. The collection is a live shared i_item_collection so the inner CV binds its
        // ItemsSource straight to it.
        struct nested_source
        {
            std::string title;
            std::shared_ptr<maui::controls::i_item_collection> items;
            friend bool operator==(const nested_source& a, const nested_source& b)
            {
                return a.title == b.title && a.items == b.items;
            }
        };

        nested_collection_page()
            : sources_(std::make_shared<maui::core::observable_collection<nested_source>>(build_sources()))
        {
            page_.set_title("Nested CollectionView");
            stack_.set_spacing(12);

            // ---- the header Label: "It's CollectionViews all the way down." ----
            banner_.set_text("It's CollectionViews all the way down.");

            // ---- the OUTER CollectionView (ItemsLayout VerticalList) ----
            outer_.set_items_layout(maui::controls::linear_items_layout::create_vertical_default());

            // ---- the outer item template: the cell IS an inner collection_view (header note) ----
            auto outer_cell = maui::controls::data_template::of<maui::controls::collection_view>();

            // The XAML inner CV is a HorizontalList. ItemsLayout is a plain slot (NOT a
            // bindable_property), so it cannot be staged through the template's Values; a template SETUP
            // action carries the C# lambda template's `inner.ItemsLayout = LinearItemsLayout.Horizontal`
            // instead (data_template::add_setup) — so each realized inner CV flows its image/caption
            // items in a horizontal row, matching MAUI (the prior "horizontal CV renders vertically"
            // gap is closed).
            outer_cell->add_setup<maui::controls::collection_view>([](maui::controls::collection_view& inner) {
                inner.set_items_layout(maui::controls::linear_items_layout::create_horizontal_default());
            });

            // The inner CV's own ItemTemplate: a blue caption Label bound to gallery_item.Caption.
            auto inner_cell = maui::controls::data_template::of<maui::controls::label>();
            inner_cell->set_binding<std::string, gallery_item>(maui::controls::label::text_property(),
                                                               [](const gallery_item& item) { return item.caption; });
            inner_cell->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::blue);
            outer_cell->set_value(maui::controls::items_view::item_template_property(), inner_cell);

            // The inner CV chrome: HeightRequest=100 + both scroll bars Never (the XAML inner CV attrs).
            outer_cell->set_value(maui::controls::height_request_property(), 100.0);
            outer_cell->set_value(maui::controls::items_view::horizontal_scroll_bar_visibility_property(),
                                  maui::core::scroll_bar_visibility::never);
            outer_cell->set_value(maui::controls::items_view::vertical_scroll_bar_visibility_property(),
                                  maui::core::scroll_bar_visibility::never);

            // The inner CV's ItemsSource = the outer item's {Binding Items}: bind it off each outer
            // nested_source so every realized inner CV hosts its own source.
            outer_cell->set_binding<std::shared_ptr<maui::controls::i_item_collection>, nested_source>(
                maui::controls::items_view::items_source_property(),
                [](const nested_source& src) { return src.items; });

            // The outer item's Title is the red italic Label that sits ABOVE the inner horizontal list
            // (XAML: TextColor=Red, FontAttributes=Italic). The port's templated cell renders a single
            // root (the inner CV), so the Title surfaces as that inner CV's HEADER (header note). The
            // Header VALUE is the source Title (a boxed string bound off each outer item — so the realized
            // header's BindingContext IS that title string), and a Header TEMPLATE renders it RED + ITALIC
            // like MAUI instead of the plain headline-font default supplementary. The header label binds
            // the self path (the boxed string context = the title) and carries the red color + italic
            // slant (FontAttributes.Italic → font_slant::italic, the port convention).
            auto header_cell = maui::controls::data_template::of<maui::controls::label>();
            header_cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                               [](const std::string& title) { return title; });
            header_cell->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::red);
            header_cell->set_value(maui::controls::label::font_property(),
                                   maui::core::font::system_font_of_weight(maui::core::font_weight::regular,
                                                                           maui::core::font_slant::italic));
            outer_cell->set_binding<maui::controls::boxed_item, nested_source>(
                maui::controls::structured_items_view::header_property(),
                [](const nested_source& src) { return maui::controls::boxed_item::of(src.title); });
            outer_cell->set_value(maui::controls::structured_items_view::header_template_property(), header_cell);

            outer_.set_item_template(outer_cell);

            // ---- ItemsSource = the view model's 20 sources ----
            outer_.set_items_source(sources_);

            stack_.add(banner_);
            stack_.add(outer_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& banner()
        {
            return banner_;
        }
        [[nodiscard]] maui::controls::collection_view& outer()
        {
            return outer_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<nested_source>>& sources() const
        {
            return sources_;
        }

    private:
        // NestedCollectionViewModel(): 20 NestedItemSource("Source 0".."Source 19"), each with 6..15
        // gallery_item captions drawn from a DemoFilteredItemSource (deterministic here — the port's
        // seeded-RNG convention so the per-source counts are reproducible).
        [[nodiscard]] std::vector<nested_source> build_sources()
        {
            // DemoFilteredItemSource images (NestedCollectionModel.cs Images[]) cycled by item index.
            static constexpr const char* images[] = {"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                     "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            std::vector<nested_source> sources;
            for (int n = 0; n < 20; ++n)
            {
                const int count = 6 + (n % 9); // NestedCollectionViewModel: deterministic 6..14 by source index
                std::vector<gallery_item> inner;
                inner.reserve(static_cast<std::size_t>(count));
                for (int k = 0; k < count; ++k)
                {
                    // caption "{image}, {k}" (NestedItemSource ctor: Images[k % 7] + ", " + k)
                    inner.push_back(gallery_item{std::string(images[k % 7]) + ", " + std::to_string(k)});
                }
                auto inner_collection =
                    std::make_shared<maui::core::observable_collection<gallery_item>>(std::move(inner));
                inner_sources_.push_back(inner_collection); // pin each live inner source for the page lifetime
                sources.push_back(nested_source{"Source " + std::to_string(n),
                                                maui::controls::make_item_collection(std::move(inner_collection))});
            }
            return sources;
        }

        // The pinned inner collections (each {Binding Items}), kept alive for the page's lifetime so the
        // bound inner sources stay valid (§8 — the inner CVs bind against these).
        std::vector<std::shared_ptr<maui::core::observable_collection<gallery_item>>> inner_sources_;
        std::shared_ptr<maui::core::observable_collection<nested_source>> sources_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label banner_;
        maui::controls::collection_view outer_;
    };
} // namespace maui::samples
