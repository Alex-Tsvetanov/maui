#pragma once
// maui::samples::header_footer_grid_page — ports HeaderFooterGrid.xaml (+ HeaderFooterGrid.xaml.cs) of
// the C# CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries).
//
// The original page (HeaderFooterGrid): a two-row Grid — a horizontal StackLayout of two buttons
// ("Toggle Header" / "Toggle Footer") on top, and below it a CollectionView whose ItemsLayout is a
// GridItemsLayout (Span=3, Vertical, HorizontalItemSpacing=4, VerticalItemSpacing=2). The
// CollectionView.Header and CollectionView.Footer are each a VIEW (a StackLayout carrying an Image,
// a bold AntiqueWhite Label — "This Is A Header" / "This Is A Footer" — and an "Add Content" Button).
// The xaml.cs seeds the source with DemoFilteredItemSource(10), sets ItemTemplate =
// ExampleTemplates.PhotoTemplate(), and wires three handlers:
//   - ToggleHeader / ToggleFooter stash the current Header/Footer and flip it to null and back
//     (`header = CollectionView.Header ?? header; CollectionView.Header = Header==null ? header : null`);
//   - AddContentClicked appends a `Label { Text = "Grow" }` to the StackLayout that owns the tapped
//     button (the header or footer chrome grows in place).
//
// This is the VIEW-as-chrome arm of the Header/Footer trio (HeaderFooterString boxes strings,
// HeaderFooterTemplate boxes DataTemplates, this boxes a live View): the headless collection_view
// handler's realize_supplemental takes the `value.as_bindable()` branch (reuse_id "view") and hosts the
// StackLayout directly outside the scroll extent — exactly the C# `Header is View` path.
//
// The port mirrors the shape code-first:
//   - the two header/footer StackLayouts are built as a horizontal_stack_layout member each (vertical
//     stacking has no headless-visible difference for chrome, but C# uses the default StackLayout =
//     vertical; the port uses vertical_stack_layout to match the default StackOrientation) carrying an
//     image, the bold AntiqueWhite caption label, and the "Add Content" button;
//   - the GridItemsLayout(Span=3, …) is set as the collection_view's ItemsLayout;
//   - the item template is the PhotoTemplate caption Label (Text bound to each row's caption — the
//     image half has no headless-safe analog without an asset pipeline, see note);
//   - toggle_header()/toggle_footer() reproduce the stash-and-flip logic; add_content() appends a "Grow"
//     label to a chrome stack.
//
// The page OWNS its whole element tree (the items_page pattern); the generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the tree.
//
// note: ExampleTemplates.PhotoTemplate() pairs an Image (bound to "Image") above a caption Label (bound
//       to "Caption"). The port item cell is the caption Label only — an Image row would need an
//       i_image_source the headless backend can't resolve; the caption carries the image file name.
//       The header/footer Images ARE built (file_image_source over the oasis.jpg asset name) so the
//       chrome's structure matches the oracle, but the headless image handler only resolves a real
//       on-disk asset — absent one it stays a sized placeholder (the structure is what the demo shows).

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp" // maui::controls::margin_property()
#include "maui/core/aspect.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class header_footer_grid_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds
        // (CollectionViewGalleryTestItem.Caption, reduced to what this page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        header_footer_grid_page() : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items()))
        {
            page_.set_title("Header/Footer (grid)");

            // ---- the two-row outer Grid: Auto (the toggle buttons) over Auto (the CollectionView) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());

            // ---- the horizontal StackLayout of toggle buttons (Grid row 0) ----
            toggle_header_button_.set_text("Toggle Header");
            toggle_header_button_.clicked.connect([this] { toggle_header(); });
            toggle_footer_button_.set_text("Toggle Footer");
            toggle_footer_button_.clicked.connect([this] { toggle_footer(); });
            toggles_.add(toggle_header_button_);
            toggles_.add(toggle_footer_button_);

            // ---- the CollectionView (Grid row 1) with a GridItemsLayout(Span=3) ----
            // This code-first page MIRRORS the shared-XAML twin (header_footer_grid.xaml), which the two-
            // framework board compares against MAUI. That twin uses the STRING form `ItemsLayout="VerticalGrid, 3"`
            // — the loader has no <GridItemsLayout> element-form support (register_xaml_items.cpp: needs the
            // [Parameter("Orientation")] ctor-arg reflection the port lacks), so it and MAUI both render with
            // the DEFAULT item spacing (0). The original C# HeaderFooterGrid.xaml used the element form with
            // HorizontalItemSpacing="4" VerticalItemSpacing="2"; setting those here made the code-first grid
            // rows 2pt taller than the twin (measured: pitch 93px vs 87px @3x → the cpp-only iOS red). Mirror
            // the twin's string form — default spacing — so all three columns match. (If the loader ever gains
            // element-form spacing, upgrade the twin + restore 4/2 on both sides.)
            auto grid_layout = std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::vertical);
            list_.set_items_layout(grid_layout);

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            cell->set_value(maui::controls::margin_property(),
                            maui::core::thickness{6}); // <Label Margin="6"> (shared XAML)
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the VIEW Header + VIEW Footer (the whole point of this oracle page) ----
            build_header_chrome();
            build_footer_chrome();
            list_.set_header(maui::controls::boxed_item::of(header_chrome_));
            list_.set_footer(maui::controls::boxed_item::of(footer_chrome_));

            grid_.set_row(toggles_, 0);
            grid_.add(toggles_);
            grid_.set_row(list_, 1);
            grid_.add(list_);
            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }

        // HeaderFooterGrid.ToggleHeader: stash the current header, then flip it to null / back
        // (`header = CollectionView.Header ?? header; Header = Header==null ? header : null`).
        void toggle_header()
        {
            if (list_.header().has_value())
            {
                stashed_header_ = list_.header();
                list_.set_header(maui::controls::boxed_item{}); // null
            }
            else
            {
                list_.set_header(stashed_header_);
            }
        }

        // HeaderFooterGrid.ToggleFooter: the footer twin of toggle_header.
        void toggle_footer()
        {
            if (list_.footer().has_value())
            {
                stashed_footer_ = list_.footer();
                list_.set_footer(maui::controls::boxed_item{}); // null
            }
            else
            {
                list_.set_footer(stashed_footer_);
            }
        }

        // HeaderFooterGrid.AddContentClicked: append a `Label { Text = "Grow" }` to a chrome stack (the C#
        // `ve.Parent is StackLayout sl => sl.Children.Add(new Label{Text="Grow"})`). The grown labels are
        // owned here so they outlive the call (the stack only borrows i_view&).
        void add_content(maui::controls::vertical_stack_layout& chrome)
        {
            auto grow = std::make_shared<maui::controls::label>();
            grow->set_text("Grow");
            chrome.add(*grow);
            grown_labels_.push_back(std::move(grow));
        }

    private:
        // DemoFilteredItemSource(10).AddItems: ten rows, captioned "<image>, <n>" off the image ring.
        [[nodiscard]] static std::vector<demo_item> seed_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                         "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 10; ++n)
            {
                rows.push_back(
                    demo_item{images[static_cast<std::size_t>(n) % images.size()] + ", " + std::to_string(n)});
            }
            return rows;
        }

        // The header StackLayout: a 60-high oasis.jpg Image, a bold 36-pt AntiqueWhite centered caption,
        // and an "Add Content" button (which grows the header in place).
        void build_header_chrome()
        {
            header_chrome_ = std::make_shared<maui::controls::vertical_stack_layout>();
            header_chrome_->set_background(std::make_shared<maui::graphics::solid_paint>(
                maui::graphics::colors::transparent)); // BackgroundColor="Transparent"

            header_image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));
            header_image_.set_aspect(maui::core::aspect::aspect_fill);
            header_image_.set_height_request(60);

            header_label_.set_text("This Is A Header");
            header_label_.set_text_color(maui::graphics::colors::antique_white);
            header_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            header_label_.set_font(maui::core::font::system_font_of_size(36, maui::core::font_weight::bold));

            header_add_button_.set_text("Add Content");
            header_add_button_.clicked.connect([this] { add_content(*header_chrome_); });

            header_chrome_->add(header_image_);
            header_chrome_->add(header_label_);
            header_chrome_->add(header_add_button_);
        }

        // The footer StackLayout: an 80-high oasis.jpg Image, a bold 20-pt AntiqueWhite caption rotated
        // 10 degrees, and an "Add Content" button.
        void build_footer_chrome()
        {
            footer_chrome_ = std::make_shared<maui::controls::vertical_stack_layout>();
            footer_chrome_->set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::transparent));

            footer_image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));
            footer_image_.set_aspect(maui::core::aspect::aspect_fill);
            footer_image_.set_height_request(80);

            footer_label_.set_text("This Is A Footer");
            footer_label_.set_text_color(maui::graphics::colors::antique_white);
            footer_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            footer_label_.set_rotation(10); // Rotation="10"
            footer_label_.set_font(maui::core::font::system_font_of_size(20, maui::core::font_weight::bold));

            footer_add_button_.set_text("Add Content");
            footer_add_button_.clicked.connect([this] { add_content(*footer_chrome_); });

            footer_chrome_->add(footer_image_);
            footer_chrome_->add(footer_label_);
            footer_chrome_->add(footer_add_button_);
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::grid grid_;

        // row 0: the toggle buttons + their horizontal stack
        maui::controls::horizontal_stack_layout toggles_;
        maui::controls::button toggle_header_button_;
        maui::controls::button toggle_footer_button_;

        // row 1: the collection_view
        maui::controls::collection_view list_;

        // the VIEW header chrome (owned via shared_ptr so a boxed_item can keep it alive across toggles)
        std::shared_ptr<maui::controls::vertical_stack_layout> header_chrome_;
        maui::controls::image header_image_;
        maui::controls::label header_label_;
        maui::controls::button header_add_button_;

        // the VIEW footer chrome
        std::shared_ptr<maui::controls::vertical_stack_layout> footer_chrome_;
        maui::controls::image footer_image_;
        maui::controls::label footer_label_;
        maui::controls::button footer_add_button_;

        // the stashed chrome a toggle flips back to (HeaderFooterGrid's `header`/`footer` fields)
        maui::controls::boxed_item stashed_header_;
        maui::controls::boxed_item stashed_footer_;

        // the "Grow" labels add_content mints (kept alive — the stack borrows i_view&)
        std::vector<std::shared_ptr<maui::controls::label>> grown_labels_;
    };
} // namespace maui::samples
