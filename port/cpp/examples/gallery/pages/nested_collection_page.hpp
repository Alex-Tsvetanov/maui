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
#include "maui/controls/grid.hpp"
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
#include "maui/core/grid_length.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

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

        // The OUTER cell — a 2-COLUMN Grid reproducing MAUI's shipped render: the red-italic Title in a
        // width-Auto column 0, the inner HORIZONTAL CollectionView (its image/caption items) in the star
        // column 1, so the Title sits on the LEFT and the captions flow to its RIGHT on the same top line
        // (measured off the Android maui capture — Title at x~0, first caption at x~146). This replaces the
        // earlier "Title as the inner CV's Header" reduction: the port renders a horizontal CV's Header as a
        // full-width TOP band (the wave-27 no-horizontal-scroll deviation, kept for header_footer_grid_
        // horizontal), which put the Title ABOVE the captions instead of beside them. Carrying the Title in
        // its own Grid column — NOT the CV Header — sidesteps that band model and matches MAUI. Mirrors the
        // header_footer_template.hpp `photo_cell : grid` pattern; default-constructible so a
        // data_template::of<source_cell>() activates it; its handler is the shared layout_handler
        // (registered in register_handlers). on_binding_context_changed pushes the bound Title + inner items.
        class source_cell : public maui::controls::grid
        {
        public:
            source_cell()
            {
                add_column_definition(maui::core::grid_length::automatic()); // col 0: the Title (Auto width)
                add_column_definition(maui::core::grid_length::star());      // col 1: the inner horizontal CV
                set_column_spacing(4); // caption lands just right of the Title (~x146 in the MAUI render)

                // col 0 — the red italic Title (XAML TextColor=Red / FontAttributes=Italic), top-aligned.
                title_.set_text_color(maui::graphics::colors::red);
                title_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::regular,
                                                                        maui::core::font_slant::italic));
                title_.set_vertical_layout_alignment(maui::core::layout_alignment::start);
                add(title_); // column 0 (default)

                // col 1 — the inner HORIZONTAL CollectionView: HeightRequest=100, both scroll bars Never, an
                // ItemTemplate of a blue 10pt caption Label bound to gallery_item.caption (the XAML inner CV).
                inner_.set_items_layout(maui::controls::linear_items_layout::create_horizontal_default());
                inner_.set_height_request(100);
                inner_.set_vertical_layout_alignment(maui::core::layout_alignment::start);
                inner_.set_horizontal_scroll_bar_visibility(maui::core::scroll_bar_visibility::never);
                inner_.set_vertical_scroll_bar_visibility(maui::core::scroll_bar_visibility::never);
                auto caption = maui::controls::data_template::of<maui::controls::label>();
                caption->set_binding<std::string, gallery_item>(maui::controls::label::text_property(),
                                                                [](const gallery_item& it) { return it.caption; });
                caption->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::blue);
                caption->set_value(maui::controls::label::font_property(),
                                   maui::core::font::system_font_of_size(10)); // XAML inner Label FontSize="10"
                inner_.set_item_template(caption);
                add(inner_);
                set_column(inner_, 1);
            }

        protected:
            // The realize path sets this cell's BindingContext to the outer nested_source; push its Title to
            // the Label and its Items to the inner CV's ItemsSource (so each realized inner CV hosts its own).
            void on_binding_context_changed() override
            {
                maui::controls::grid::on_binding_context_changed(); // propagate to children first
                if (const auto src = binding_context<nested_source>())
                {
                    title_.set_text(src->title);
                    inner_.set_items_source(src->items);
                }
            }

        private:
            maui::controls::label title_;
            maui::controls::collection_view inner_;
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

            // ---- the outer item template: a source_cell (Title in col 0, inner horizontal CV in col 1) ----
            // Each realized source_cell binds its Title + inner ItemsSource off the outer nested_source in
            // source_cell::on_binding_context_changed — so the Title sits LEFT of the captions on one line,
            // matching MAUI (see the source_cell doc comment for why the prior Title-as-CV-Header rendered
            // the Title as a top band instead).
            outer_.set_item_template(maui::controls::data_template::of<source_cell>());

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

        // PRE-MOUNT hook (gallery_host.hpp gallery_pre_mount detects it via `requires`): register the
        // source_cell handler BEFORE the collection_view realize walks the tree. source_cell is a brand-new
        // user type (like header_footer_template's photo_cell), so of<source_cell>() → create_handler resolves
        // it through THIS app's per-app handler_registry — without this it falls back to the text mirror and
        // the captions/inner CV go missing. source_cell is a grid subclass, so it uses the layout_handler.
        void register_handlers(maui::hosting::maui_app& app)
        {
            maui::core::register_handler<source_cell, maui::core::layout_handler>(app.handlers());
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
