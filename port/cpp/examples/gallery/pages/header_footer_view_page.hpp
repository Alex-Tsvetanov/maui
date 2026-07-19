#pragma once
// maui::samples::header_footer_view_page — ports the shared twin header_footer_view.xaml
// (port/maui-reference/pages/header_footer_view.xaml — the HeaderFooterView.xaml oracle).
//
// The original page (HeaderFooterView): a CollectionView whose Header and Footer are each a VIEW
// (a Grid, NOT a string and NOT a DataTemplate):
//   - Header: a Grid with an oasis.jpg Image (100 high) and a centered bold AntiqueWhite Label bound to
//     {Binding HeaderText} ("This Is A Header");
//   - Footer: a 2x2 Grid with a cover1.jpg Image + a rotated bold AntiqueWhite Label bound to
//     {Binding FooterText} ("This Is A Footer") spanning the top row, and two command-bound Buttons on
//     the bottom row — "Add 2 Items" (Command="{Binding AddCommand}") and "Clear All Items"
//     (Command="{Binding ClearCommand}").
// The xaml.cs sets ItemTemplate = ExampleTemplates.PhotoTemplate() and CollectionView.BindingContext =
// a HeaderFooterViewModel(0) (a DemoFilteredItemSource subclass: count 0 ⇒ an EMPTY source initially).
// The viewmodel's AddCommand awaits ~1s then AddItems(Items, 2); ClearCommand clears Items; HeaderText /
// FooterText are the two static caption strings.
//
// CollectionView.Header / .Footer (the VIEW form) are UNSUPPORTED by the port (see the shared XAML's
// own comment), so — exactly like the twin — the header Grid and footer Grid are plain SIBLINGS of the
// CollectionView inside a VerticalStackLayout, in top-to-bottom resting order: header Grid, (empty)
// CollectionView, footer Grid. They are NOT boxed onto CollectionView.Header/Footer (that was this
// page's prior, now-corrected, structure — it made both chrome grids overlap at the top of the page).
//
// The port mirrors the shape code-first:
//   - the Header is a grid (Image + bound-text Label), a sibling ABOVE the CollectionView;
//   - the Footer is a grid (Image + rotated bound-text Label spanning the top + two command Buttons on
//     the bottom), a sibling BELOW the CollectionView;
//   - the AddCommand / ClearCommand are wired to each button's command — add_items() appends 2 rows,
//     clear_items() empties the live source (so the source starts empty and the footer buttons drive it);
//   - the item template is the PhotoTemplate caption Label (Text bound to each row's caption).
//
// The page OWNS its whole element tree; the generic mount (app_host.hpp) attaches every owned view's
// handler and hosts the tree.
//
// note: HeaderText / FooterText bind against the CollectionView's BindingContext (the viewmodel). Since
//       the Header/Footer here are VIEWS (hosted directly, not templated against a per-item context), the
//       port sets the two caption Labels to the same constant strings the viewmodel returns ("This Is A
//       Header" / "This Is A Footer") — the bound text is constant, so the rendered text is identical.
//       The C# AddCommand awaits Task.Delay(1s) before adding; the port adds synchronously (no event loop
//       in the headless sim) — the 2-item add itself is the demonstrated behavior. The Images are built
//       (file_image_source over the asset names) for structural fidelity but the headless image handler
//       only resolves a real on-disk asset (absent one they stay sized placeholders). The item
//       PhotoTemplate is the caption Label only (no per-row Image), as in the sibling demos.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class header_footer_view_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds.
        struct demo_item
        {
            std::string caption;
        };

        header_footer_view_page()
            // HeaderFooterViewModel(0): count 0 ⇒ the source starts EMPTY (the footer buttons fill it).
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(std::vector<demo_item>{}))
        {
            page_.set_title("Header/Footer (view)");

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the header Grid + footer Grid (plain siblings — CollectionView.Header/Footer view
            // form is unsupported, per the shared XAML's own comment) ----
            build_header_chrome();
            build_footer_chrome();

            // Twin structure: VerticalStackLayout > [header Grid, (empty) CollectionView, footer Grid].
            root_.add(header_chrome_);
            root_.add(list_);
            root_.add(footer_chrome_);
            page_.set_content(root_);
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
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }

        // HeaderFooterViewModel.AddCommand: AddItems(Items, 2) — append two rows off the image ring
        // (continuing the index sequence, like DemoFilteredItemSource.AddItems). The C# command awaits a
        // 1s delay first; the port adds synchronously (no headless event loop) — see header note.
        void add_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg",      "photo.jpg",  "Vegetables.jpg",
                                                         "Fruits.jpg", "FlowerBuds.jpg", "Legumes.jpg"};
            const int base = next_index_;
            for (int k = 0; k < 2; ++k)
            {
                const int n = base + k;
                items_->add(demo_item{images[static_cast<std::size_t>(n) % images.size()] + ", " + std::to_string(n)});
            }
            next_index_ = base + 2;
        }

        // HeaderFooterViewModel.ClearCommand: Items.Clear().
        void clear_items()
        {
            items_->clear();
        }

    private:
        // The Header Grid: an oasis.jpg Image (100 high) overlaid by a centered bold AntiqueWhite Label
        // bound to {Binding HeaderText} ("This Is A Header" — the constant the viewmodel returns).
        void build_header_chrome()
        {
            header_image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));
            header_image_.set_aspect(maui::core::aspect::aspect_fill);
            header_image_.set_height_request(100);

            header_label_.set_text("This Is A Header"); // {Binding HeaderText}
            header_label_.set_text_color(maui::graphics::colors::antique_white);
            header_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            header_label_.set_font(maui::core::font::system_font_of_size(36, maui::core::font_weight::bold));

            header_chrome_.add(header_image_); // both occupy the single implicit cell (overlaid, like XAML)
            header_chrome_.add(header_label_);
        }

        // The Footer 2x2 Grid: row 0 (spanning both columns) a cover1.jpg Image (80 high) overlaid by a
        // rotated bold AntiqueWhite Label bound to {Binding FooterText}; row 1 two command Buttons —
        // "Add 2 Items" (AddCommand) and "Clear All Items" (ClearCommand).
        void build_footer_chrome()
        {
            footer_chrome_.add_row_definition(maui::core::grid_length::star());
            footer_chrome_.add_row_definition(maui::core::grid_length::star());
            footer_chrome_.add_column_definition(maui::core::grid_length::star());
            footer_chrome_.add_column_definition(maui::core::grid_length::star());

            footer_image_.set_source(maui::controls::image_source::from_file("cover1.jpg"));
            footer_image_.set_aspect(maui::core::aspect::aspect_fill);
            footer_image_.set_height_request(80);

            footer_label_.set_text("This Is A Footer"); // {Binding FooterText}
            footer_label_.set_text_color(maui::graphics::colors::antique_white);
            footer_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            footer_label_.set_rotation(10); // Rotation="10"
            footer_label_.set_font(maui::core::font::system_font_of_size(20, maui::core::font_weight::bold));

            add_button_.set_text("Add 2 Items");
            add_button_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::center);     // HorizontalOptions="Center"
            add_button_.command = [this] { add_items(); }; // Command="{Binding AddCommand}"
            clear_button_.set_text("Clear All Items");
            clear_button_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::center);         // HorizontalOptions="Center"
            clear_button_.command = [this] { clear_items(); }; // Command="{Binding ClearCommand}"

            // Grid.Row="0" Grid.ColumnSpan="2" for the image + label; the buttons on Row 1, Cols 0/1.
            footer_chrome_.add(footer_image_);
            footer_chrome_.set_row(footer_image_, 0);
            footer_chrome_.set_column_span(footer_image_, 2);
            footer_chrome_.add(footer_label_);
            footer_chrome_.set_row(footer_label_, 0);
            footer_chrome_.set_column_span(footer_label_, 2);
            footer_chrome_.add(add_button_);
            footer_chrome_.set_row(add_button_, 1);
            footer_chrome_.set_column(add_button_, 0);
            footer_chrome_.add(clear_button_);
            footer_chrome_.set_row(clear_button_, 1);
            footer_chrome_.set_column(clear_button_, 1);
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        int next_index_ = 0;                                                  // continues the AddItems index sequence
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_; // the twin's outer VerticalStackLayout
        maui::controls::collection_view list_;       // empty — no Header/Footer set (unsupported view form)

        // the header Grid (a plain sibling ABOVE the CollectionView)
        maui::controls::grid header_chrome_;
        maui::controls::image header_image_;
        maui::controls::label header_label_;

        // the footer Grid (a plain sibling BELOW the CollectionView)
        maui::controls::grid footer_chrome_;
        maui::controls::image footer_image_;
        maui::controls::label footer_label_;
        maui::controls::button add_button_;
        maui::controls::button clear_button_;
    };
} // namespace maui::samples
