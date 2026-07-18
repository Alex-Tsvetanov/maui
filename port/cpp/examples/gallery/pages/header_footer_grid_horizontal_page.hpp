#pragma once
// maui::samples::header_footer_grid_horizontal_page — ports the shared twin
// header_footer_grid_horizontal.xaml (port/maui-reference/pages/header_footer_grid_horizontal.xaml —
// the HeaderFooterGridHorizontal.xaml oracle).
//
// The original page is the HORIZONTAL twin of HeaderFooterGrid: a two-row Grid — a horizontal
// StackLayout of two buttons ("Toggle Header" / "Toggle Footer") on top, and below it (Grid row 1) a
// VerticalStackLayout wrapping, top to bottom: the header StackLayout, a CollectionView (ItemsLayout a
// GridItemsLayout Span=3 **Orientation=Horizontal**), and the footer StackLayout. The header/footer
// StackLayouts each carry an Image, a bold AntiqueWhite caption Label ("This Is A Header" / "This Is A
// Footer"), and a nested horizontal StackLayout with an "Add Content" Button.
//
// CollectionView.Header / .Footer (the VIEW form) are UNSUPPORTED by the port (see the shared XAML's own
// comment), so — exactly like the twin — the header/footer StackLayouts are moved to plain SIBLINGS
// ABOVE/BELOW the CollectionView inside a VerticalStackLayout, in top-to-bottom resting order, rather
// than boxed onto CollectionView.Header/Footer (that was this page's prior, now-corrected, structure —
// it squeezed both chrome stacks into item-cell-sized space and stacked them behind the header image).
// The toggle buttons therefore no longer toggle a CollectionView.Header/Footer value; they are kept as
// inert resting UI (Clicked handlers omitted, matching the shared XAML's own "Clicked handlers omitted"
// note) — there is nothing left for them to toggle now that the chrome is plain siblings, matching the
// oracle's resting (never-toggled) capture.
//
// The port mirrors the shape code-first:
//   - the two header/footer chrome StackLayouts are built as vertical_stack_layout members (C#'s
//     default StackLayout orientation is vertical) carrying an image, the bold AntiqueWhite caption
//     label, and a nested horizontal_stack_layout holding the "Add Content" button;
//   - the GridItemsLayout(Span=3, **Horizontal**, …) is set as the collection_view's ItemsLayout;
//   - the item template is the PhotoTemplate caption Label (Text bound to each row's caption — the
//     image half has no headless-safe analog without an asset pipeline, see note).
//
// The page OWNS its whole element tree (the items_page pattern); the generic mount (app_host.hpp) attaches
// every owned view's handler and hosts the tree.
//
// note: ExampleTemplates.PhotoTemplate() pairs an Image (bound to "Image") above a caption Label (bound
//       to "Caption"). The port item cell is the caption Label only — an Image row would need an
//       i_image_source the headless backend can't resolve; the caption carries the image file name.
//       The header/footer Images ARE built (file_image_source over the oasis.jpg / cover1.jpg asset
//       names) so the chrome's structure matches the oracle, but the headless image handler only
//       resolves a real on-disk asset — absent one it stays a sized placeholder (the structure is what
//       the demo shows).

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
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

namespace maui::samples
{
    class header_footer_grid_horizontal_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds
        // (CollectionViewGalleryTestItem.Caption, reduced to what this page surfaces).
        struct demo_item
        {
            std::string caption;
        };

        header_footer_grid_horizontal_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items()))
        {
            page_.set_title("Header/Footer (grid, horizontal)");

            // ---- the two-row outer Grid: Auto (the toggle buttons) over Auto (the CollectionView) ----
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());

            // ---- the horizontal StackLayout of toggle buttons (Grid row 0; resting UI — Clicked
            // handlers omitted, matching the shared XAML's own note) ----
            toggle_header_button_.set_text("Toggle Header");
            toggle_footer_button_.set_text("Toggle Footer");
            toggles_.add(toggle_header_button_);
            toggles_.add(toggle_footer_button_);

            // ---- the CollectionView with a HORIZONTAL GridItemsLayout(Span=3) ----
            // Mirror the shared-XAML twin's STRING form `ItemsLayout="HorizontalGrid, 3"` (default spacing) —
            // see header_footer_grid_page.hpp for the full rationale: the loader has no <GridItemsLayout>
            // element-form support, so MAUI + xaml render spacing 0; setting the original C#'s 4/2 here made
            // the code-first grid diverge from the twin the board compares against.
            auto grid_layout = std::make_shared<maui::controls::grid_items_layout>(
                3, maui::controls::items_layout_orientation::horizontal);
            list_.set_items_layout(grid_layout);

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            cell->set_value(maui::controls::margin_property(),
                            maui::core::thickness{6}); // <Label Margin="6"> (shared XAML)
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the header/footer chrome StackLayouts (plain siblings — CollectionView.Header/Footer
            // view form is unsupported, per the shared XAML's own comment) ----
            build_header_chrome();
            build_footer_chrome();

            // Twin structure: Grid row 1 = VerticalStackLayout > [header stack, CollectionView, footer
            // stack].
            content_.add(header_chrome_);
            content_.add(list_);
            content_.add(footer_chrome_);

            grid_.set_row(toggles_, 0);
            grid_.add(toggles_);
            grid_.set_row(content_, 1);
            grid_.add(content_);
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

        // AddContentClicked: append a `Label { Text = "Grow" }` to the inner horizontal StackLayout the
        // tapped button sits in (the C# `ve.Parent is StackLayout sl => sl.Children.Add(new
        // Label{Text="Grow"})`). The grown labels are owned here so they outlive the call (the stack only
        // borrows i_view&).
        void add_content(maui::controls::horizontal_stack_layout& button_row)
        {
            auto grow = std::make_shared<maui::controls::label>();
            grow->set_text("Grow");
            button_row.add(*grow);
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
        // and a nested horizontal StackLayout holding the "Add Content" button (which grows that inner
        // stack in place).
        void build_header_chrome()
        {
            header_image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));
            header_image_.set_aspect(maui::core::aspect::aspect_fill);
            header_image_.set_height_request(60);

            header_label_.set_text("This Is A Header");
            header_label_.set_text_color(maui::graphics::colors::antique_white);
            header_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            header_label_.set_font(maui::core::font::system_font_of_size(36, maui::core::font_weight::bold));

            header_add_button_.set_text("Add Content");
            header_add_button_.clicked.connect([this] { add_content(header_button_row_); });
            header_button_row_.add(header_add_button_);

            header_chrome_.add(header_image_);
            header_chrome_.add(header_label_);
            header_chrome_.add(header_button_row_);
        }

        // The footer StackLayout: an 80-high cover1.jpg Image, a bold 20-pt AntiqueWhite caption rotated
        // 10 degrees, and a nested horizontal StackLayout holding the "Add Content" button.
        void build_footer_chrome()
        {
            footer_image_.set_source(maui::controls::image_source::from_file("cover1.jpg"));
            footer_image_.set_aspect(maui::core::aspect::aspect_fill);
            footer_image_.set_height_request(80);

            footer_label_.set_text("This Is A Footer");
            footer_label_.set_text_color(maui::graphics::colors::antique_white);
            footer_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            footer_label_.set_rotation(10); // Rotation="10"
            footer_label_.set_font(maui::core::font::system_font_of_size(20, maui::core::font_weight::bold));

            footer_add_button_.set_text("Add Content");
            footer_add_button_.clicked.connect([this] { add_content(footer_button_row_); });
            footer_button_row_.add(footer_add_button_);

            footer_chrome_.add(footer_image_);
            footer_chrome_.add(footer_label_);
            footer_chrome_.add(footer_button_row_);
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::grid grid_;

        // row 0: the toggle buttons + their horizontal stack
        maui::controls::horizontal_stack_layout toggles_;
        maui::controls::button toggle_header_button_;
        maui::controls::button toggle_footer_button_;

        // row 1: VerticalStackLayout > [header stack, CollectionView, footer stack]
        maui::controls::vertical_stack_layout content_;
        maui::controls::collection_view list_;

        // the header chrome (a plain sibling ABOVE the CollectionView)
        maui::controls::vertical_stack_layout header_chrome_;
        maui::controls::image header_image_;
        maui::controls::label header_label_;
        maui::controls::horizontal_stack_layout header_button_row_; // the C# inner button StackLayout
        maui::controls::button header_add_button_;

        // the footer chrome (a plain sibling BELOW the CollectionView)
        maui::controls::vertical_stack_layout footer_chrome_;
        maui::controls::image footer_image_;
        maui::controls::label footer_label_;
        maui::controls::horizontal_stack_layout footer_button_row_;
        maui::controls::button footer_add_button_;

        // the "Grow" labels add_content mints (kept alive — the stack borrows i_view&)
        std::vector<std::shared_ptr<maui::controls::label>> grown_labels_;
    };
} // namespace maui::samples
