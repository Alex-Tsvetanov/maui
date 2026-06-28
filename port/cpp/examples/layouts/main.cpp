// layouts — composing a UI from layout containers, with the idiomatic maui::ui builder.
//
// ONE concept: how MAUI arranges children. Two layouts cover the common cases:
//   - ui::vstack stacks children top-to-bottom with a uniform .spacing(...).
//   - ui::grid places children in a matrix; .columns(...)/.rows(...) size the tracks with grid_length
//     (ui::absolute(n) / ui::automatic() (fit) / ui::star() (share)), and .cell(row, col, child) positions
//     each child (the Grid.Row / Grid.Column attached properties). .padding(...) insets a layout's content.
// ui::app owns the window + tree, so this app declares no members at all.
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

namespace ui = maui::ui;

class layouts_app : public ui::app
{
public:
    layouts_app()
    {
        set_content(ui::page(ui::vstack(ui::label("Layouts demo"), ui::label("A vertical stack above a 2x2 grid"),
                                        // a 2x2 grid: two Star columns (split the width), two Auto rows (fit)
                                        ui::grid()
                                            .columns(ui::star(), ui::star())
                                            .rows(ui::automatic(), ui::automatic())
                                            .row_spacing(8)
                                            .column_spacing(8)
                                            .cell(0, 0, ui::label("(0,0)"))
                                            .cell(0, 1, ui::label("(0,1)"))
                                            .cell(1, 0, ui::label("(1,0)"))
                                            .cell(1, 1, ui::label("(1,1)")))
                                 .spacing(12)
                                 .padding(ui::thickness{16.0})));
        set_title("Layouts");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<layouts_app>();
    return builder;
}
