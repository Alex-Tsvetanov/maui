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
//       the inner collection_view, and the outer item's Title surfaces as that inner CV's HEADER (a
//       boxed string bound off the same outer item) — the title text is preserved adjacent to its inner
//       list, just hosted as the inner list's header rather than a separate Grid row (documented
//       reduction; the bound data — Title + the inner image/caption captions — is identical). The
//       headless virtualization sim realizes the OUTER cells (each an inner collection_view bound to its
//       source); headless has no live native pump to recursively realize each inner CV's own cells, so
//       there the nesting is wired + bound but the inner cells' geometry is not arranged. On the iOS
//       backend the inner UICollectionViews DO recursively realize + self-size their own cells (the
//       captions render), so the full nesting is visible; the one remaining gap is the inner CV's
//       HORIZONTAL orientation (the documented items_layout-non-bindable limitation above) — the inner
//       cells flow vertically rather than in a horizontal row.

#include <memory>
#include <random>
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
#include "maui/core/observable_collection.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

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

            // note: the XAML inner CV is a HorizontalList. ItemsLayout is exposed as a non-bindable
            //       shared_ptr (StructuredItemsView.ItemsLayout), so a data_template can only stage the
            //       BINDABLE inner-CV chrome (ItemTemplate, the two scroll-bar visibilities, HeightRequest,
            //       ItemsSource, Header) — the HorizontalList ORIENTATION cannot be pushed through the
            //       template, so each inner CV keeps its default vertical layout (documented gap; every
            //       other inner-CV attribute is reproduced).

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

            // The outer item's Title is preserved as the inner CV's HEADER (the red Label reduction —
            // header note): bind the inner CV's Header (a boxed string) off the same outer item, so each
            // inner list shows its source's Title above its image/caption row.
            outer_cell->set_binding<maui::controls::boxed_item, nested_source>(
                maui::controls::structured_items_view::header_property(),
                [](const nested_source& src) { return maui::controls::boxed_item::of(src.title); });

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

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, banner_, "banner_");
            gallery_attach_one(app, outer_, "outer_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // the stack hosts the banner + outer collection view
            gallery_rehost_content(page_); // the page hosts the stack
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
            std::vector<nested_source> sources;
            std::mt19937 rng(20240619); // fixed seed (DemoFilteredItemSource's Random, made deterministic)
            std::uniform_int_distribution<int> count_dist(6, 14); // C# Random.Next(6, 15) → [6,14]
            for (int n = 0; n < 20; ++n)
            {
                const int count = count_dist(rng);
                std::vector<gallery_item> inner;
                inner.reserve(static_cast<std::size_t>(count));
                for (int k = 0; k < count; ++k)
                {
                    inner.push_back(gallery_item{"Caption " + std::to_string(n) + "-" + std::to_string(k)});
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
