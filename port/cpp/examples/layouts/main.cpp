// layouts — composing a UI from layout containers.
//
// ONE concept: how MAUI arranges children. Two layouts cover the common cases:
//   - vertical_stack_layout stacks children top-to-bottom with a uniform Spacing.
//   - grid places children in a matrix of rows and columns (here a 2x2 grid). Row/column sizing uses
//     grid_length: a fixed absolute size, Auto (fit the child), or Star (share remaining space).
// Children are positioned in the grid with set_row / set_column (the Grid.Row / Grid.Column attached
// properties). Padding (a thickness) insets a layout's content; Spacing separates siblings.
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/grid_length.hpp"

class layouts_app : public maui::controls::application
{
public:
    layouts_app()
    {
        // ---- Outer container: a vertical stack with padding around its edges and spacing between rows ----
        root_.set_padding(maui::core::thickness{16.0});
        root_.set_spacing(12.0);

        title_.set_text("Layouts demo");
        subtitle_.set_text("A vertical stack above a 2x2 grid");

        // ---- A 2x2 grid: two Star columns (split the width evenly), two Auto rows (fit their content) ----
        cells_.add_column_definition(maui::core::grid_length::star());
        cells_.add_column_definition(maui::core::grid_length::star());
        cells_.add_row_definition(maui::core::grid_length::automatic());
        cells_.add_row_definition(maui::core::grid_length::automatic());
        cells_.set_row_spacing(8.0);
        cells_.set_column_spacing(8.0);

        top_left_.set_text("(0,0)");
        top_right_.set_text("(0,1)");
        bottom_left_.set_text("(1,0)");
        bottom_right_.set_text("(1,1)");

        // Add each cell to the grid, then position it with the Row/Column attached properties.
        cells_.add(top_left_);
        cells_.set_row(top_left_, 0);
        cells_.set_column(top_left_, 0);
        cells_.add(top_right_);
        cells_.set_row(top_right_, 0);
        cells_.set_column(top_right_, 1);
        cells_.add(bottom_left_);
        cells_.set_row(bottom_left_, 1);
        cells_.set_column(bottom_left_, 0);
        cells_.add(bottom_right_);
        cells_.set_row(bottom_right_, 1);
        cells_.set_column(bottom_right_, 1);

        // Stack the pieces top-to-bottom: title, subtitle, then the grid.
        root_.add(title_);
        root_.add(subtitle_);
        root_.add(cells_);

        page_.set_content(root_);
        window_.set_content(page_);
        window_.set_title("Layouts");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::vertical_stack_layout root_;
    maui::controls::label title_;
    maui::controls::label subtitle_;
    maui::controls::grid cells_;
    maui::controls::label top_left_;
    maui::controls::label top_right_;
    maui::controls::label bottom_left_;
    maui::controls::label bottom_right_;
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<layouts_app>();
    return builder;
}
