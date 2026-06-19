#pragma once
// maui::samples::grid_page — ports GridPage.xaml
//
// A self-contained, code-first demo of the Grid control. It mirrors the C# gallery page
// (Pages/Layouts/GridPage.xaml): a 3-row x 2-column grid filled with a colored BoxView + a centered Label
// in each cell, where the bottom row's pair spans both columns. It demonstrates the whole Grid contract:
//   - RowDefinitions: a 2-star row, an (implicit 1-)star row, and a fixed 100-unit row;
//   - ColumnDefinitions: two (implicit 1-)star columns;
//   - the Grid.Row / Grid.Column attached placement (each BoxView+Label pair shares one cell), and
//   - Grid.ColumnSpan="2" on the bottom row (one cell stretched across both columns).
// A bare RowDefinition / ColumnDefinition with no Height/Width is a single Star in MAUI, so the two
// "default" definitions are added as grid_length::star().
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// note: the C# Labels use HorizontalOptions/VerticalOptions="Center" to center within the cell; the port's
//       view base exposes no layout-options setter, so the labels are centered via horizontal/vertical TEXT
//       alignment instead — the closest headless-safe equivalent for the visual intent.

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class grid_page
    {
    public:
        grid_page()
        {
            page_.set_title("Grid");

            // RowDefinitions: 2* / * / 100 ; ColumnDefinitions: * / *.
            grid_.add_row_definition(maui::core::grid_length{2.0, maui::core::grid_unit_type::star}); // "2*"
            grid_.add_row_definition(maui::core::grid_length::star());                                // "" => *
            grid_.add_row_definition(maui::core::grid_length{100.0});                                 // "100" absolute
            grid_.add_column_definition(maui::core::grid_length::star());                             // "" => *
            grid_.add_column_definition(maui::core::grid_length::star());                             // "" => *

            // Row 0, Col 0 — green box + centered label.
            cell00_box_.set_color(maui::graphics::colors::green);
            add_cell(cell00_box_, cell00_label_, "Row 0, Column 0", 0, 0);

            // Row 0, Col 1 — blue box + centered label.
            cell01_box_.set_color(maui::graphics::colors::blue);
            add_cell(cell01_box_, cell01_label_, "Row 0, Column 1", 0, 1);

            // Row 1, Col 0 — teal box + centered label.
            cell10_box_.set_color(maui::graphics::colors::teal);
            add_cell(cell10_box_, cell10_label_, "Row 1, Column 0", 1, 0);

            // Row 1, Col 1 — purple box + centered label.
            cell11_box_.set_color(maui::graphics::colors::purple);
            add_cell(cell11_box_, cell11_label_, "Row1, Column 1", 1, 1);

            // Row 2, spanning both columns — red box + centered label (Grid.ColumnSpan="2").
            span_box_.set_color(maui::graphics::colors::red);
            add_cell(span_box_, span_label_, "Row 2, Columns 0 and 1", 2, 0);
            grid_.set_column_span(span_box_, 2);
            grid_.set_column_span(span_label_, 2);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the ten cell children first, then the grid, then
        // the page) so each parent can host its child's native view, then re-host the tree built in the ctor
        // (gallery_attach.hpp). The generic lambda preserves each member's concrete static type —
        // attach_handler keys on the static type, so an i_view& parameter would erase it.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, cell00_box_, "cell00_box_");
            gallery_attach_one(app, cell00_label_, "cell00_label_");
            gallery_attach_one(app, cell01_box_, "cell01_box_");
            gallery_attach_one(app, cell01_label_, "cell01_label_");
            gallery_attach_one(app, cell10_box_, "cell10_box_");
            gallery_attach_one(app, cell10_label_, "cell10_label_");
            gallery_attach_one(app, cell11_box_, "cell11_box_");
            gallery_attach_one(app, cell11_label_, "cell11_label_");
            gallery_attach_one(app, span_box_, "span_box_");
            gallery_attach_one(app, span_label_, "span_label_");
            gallery_attach_one(app, grid_, "grid_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(grid_);  // grid hosts its ten cell children
            gallery_rehost_content(page_); // page hosts the grid
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }

    private:
        // Place one BoxView + one centered Label into the same Grid cell (row, col).
        void add_cell(maui::controls::box_view& box, maui::controls::label& text, const char* caption, int row, int col)
        {
            grid_.add(box);
            grid_.set_row(box, row);
            grid_.set_column(box, col);

            text.set_text(caption);
            text.set_horizontal_text_alignment(maui::core::text_alignment::center);
            text.set_vertical_text_alignment(maui::core::text_alignment::center);
            grid_.add(text);
            grid_.set_row(text, row);
            grid_.set_column(text, col);
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::box_view cell00_box_;
        maui::controls::label cell00_label_;
        maui::controls::box_view cell01_box_;
        maui::controls::label cell01_label_;
        maui::controls::box_view cell10_box_;
        maui::controls::label cell10_label_;
        maui::controls::box_view cell11_box_;
        maui::controls::label cell11_label_;
        maui::controls::box_view span_box_;
        maui::controls::label span_label_;
    };
} // namespace maui::samples
