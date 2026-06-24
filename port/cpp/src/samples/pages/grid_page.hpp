#pragma once
// grid_page — a faithful reproduction of the maui-compare "grid" demo (ComparePages.GridPage()), the
// shipped-.NET-MAUI reference for the visual-parity comparison: a Grid (Padding 16, Row/ColumnSpacing 6)
// with RowDefinitions Auto / 80 / 80 and two Star columns, a bold "Grid (2 cols × rows)" header spanning
// both columns on row 0, and four fixed-row colored BoxViews — Red (col 0, row 1), Green (col 1, row 1),
// Blue (col 0, row 2), Orange (col 1, row 2). Kept 1:1 with the C# reference (same definitions, header,
// span, cell colors + placement) so the side-by-side gallery comparison is apples-to-apples.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class grid_page
    {
    public:
        grid_page()
        {
            page_.set_title("Grid");

            // Padding 16, RowSpacing 6, ColumnSpacing 6; rows Auto / 80 / 80; two star columns.
            grid_.set_padding(maui::core::thickness(16));
            grid_.set_row_spacing(6);
            grid_.set_column_spacing(6);
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length{80.0});
            grid_.add_row_definition(maui::core::grid_length{80.0});
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());

            // Bold header "Grid (2 cols × rows)" spanning both columns on row 0.
            header_.set_text("Grid (2 cols × rows)");
            header_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));
            grid_.add(header_);
            grid_.set_row(header_, 0);
            grid_.set_column(header_, 0);
            grid_.set_column_span(header_, 2);

            // Four colored boxes filling the fixed 80-tall cells: Red/Green on row 1, Blue/Orange on row 2.
            place(red_, maui::graphics::colors::red, 0, 1);
            place(green_, maui::graphics::colors::green, 1, 1);
            place(blue_, maui::graphics::colors::blue, 0, 2);
            place(orange_, maui::graphics::colors::orange, 1, 2);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }

    private:
        // Place a colored box into the grid at (column, row), filling its fixed-height cell.
        void place(maui::controls::box_view& box, const maui::graphics::color& color, int column, int row)
        {
            box.set_color(color);
            grid_.add(box);
            grid_.set_row(box, row);
            grid_.set_column(box, column);
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::label header_;
        maui::controls::box_view red_;
        maui::controls::box_view green_;
        maui::controls::box_view blue_;
        maui::controls::box_view orange_;
    };
} // namespace maui::samples
