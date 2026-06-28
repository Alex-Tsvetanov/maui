// XAML parity gate (PUBLIC_API_DESIGN.md §6): the SAME counter page produced three ways must yield an
// identical tree —
//   (1) the hand-written maui::ui builder (examples/counter),
//   (2) the build-time XAML codegen (maui_xaml_codegen counter.xaml -> counter_page.gen.hpp), and
//   (3) the runtime XAML loader (xaml_loader::load_into).
// This is what "parity check the XAML examples" means: the with-XAML and without-XAML twins are equivalent.

#include "counter_page.gen.hpp" // the codegen'd factory: ui::view_ref<content_page> counter_page()
#include "grid_page.gen.hpp"    // the codegen'd grid factory: ui::view_ref<content_page> grid_page()

#include "maui/ui.hpp"
#include "maui/xaml/xaml_loader.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"

#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

namespace
{
    namespace controls = maui::controls;

    struct page_shape
    {
        double spacing = 0;
        maui::core::thickness padding;
        std::string label_text;
        std::string button_text;
        friend bool operator==(const page_shape&, const page_shape&) = default;
    };

    // Normalized shape of a counter page (content_page > vertical_stack_layout > {label, button}).
    page_shape describe(controls::content_page& page)
    {
        page_shape shape;
        auto* stack = dynamic_cast<controls::vertical_stack_layout*>(page.content());
        EXPECT_NE(stack, nullptr);
        if (stack == nullptr)
        {
            return shape;
        }
        shape.spacing = stack->spacing();
        shape.padding = stack->padding();
        EXPECT_EQ(stack->count(), 2);
        if (stack->count() == 2)
        {
            shape.label_text = std::string(dynamic_cast<controls::label&>(stack->at(0)).text());
            shape.button_text = std::string(dynamic_cast<controls::button&>(stack->at(1)).text());
        }
        return shape;
    }

    constexpr std::string_view counter_markup = R"xml(
<ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <VerticalStackLayout Spacing="12" Padding="16">
        <Label Text="Count: 0" />
        <Button Text="Increment" />
    </VerticalStackLayout>
</ContentPage>)xml";

    TEST(xaml_parity, handwritten_codegen_and_runtime_agree)
    {
        // (1) hand-written builder (the without-XAML twin)
        auto hand = maui::ui::page(maui::ui::vstack(maui::ui::label("Count: 0"), maui::ui::button("Increment"))
                                       .spacing(12)
                                       .padding(maui::core::thickness{16}));

        // (2) build-time XAML codegen (the with-XAML twin, compiled)
        auto generated = counter_page();

        // (3) runtime XAML loader (the same markup, hydrated at runtime)
        controls::content_page runtime_page;
        const auto result = maui::xaml::xaml_loader::load_into(runtime_page, std::string(counter_markup));

        const page_shape hand_shape = describe(hand.impl());
        const page_shape gen_shape = describe(generated.impl());
        const page_shape runtime_shape = describe(runtime_page);

        EXPECT_EQ(hand_shape, gen_shape);     // hand-written == compile-time codegen
        EXPECT_EQ(hand_shape, runtime_shape); // hand-written == runtime loader

        // Pin the expected content, for a readable failure.
        EXPECT_EQ(gen_shape.spacing, 12.0);
        EXPECT_EQ(gen_shape.padding, (maui::core::thickness{16}));
        EXPECT_EQ(gen_shape.label_text, "Count: 0");
        EXPECT_EQ(gen_shape.button_text, "Increment");
    }

    struct grid_cell
    {
        int row = 0;
        int column = 0;
        std::string text;
        friend bool operator==(const grid_cell&, const grid_cell&) = default;
    };

    // The placed cells of a content_page whose content is a grid (row/column from the Grid.Row/Grid.Column
    // attached properties, plus the cell's label text).
    std::vector<grid_cell> describe_grid(controls::content_page& page)
    {
        std::vector<grid_cell> cells;
        auto* grid = dynamic_cast<controls::grid*>(page.content());
        EXPECT_NE(grid, nullptr);
        if (grid == nullptr)
        {
            return cells;
        }
        for (int i = 0; i < grid->count(); ++i)
        {
            maui::core::i_view& child = grid->at(i);
            cells.push_back({.row = grid->get_row(child),
                             .column = grid->get_column(child),
                             .text = std::string(dynamic_cast<controls::label&>(child).text())});
        }
        return cells;
    }

    // Grid parity: the compile-time codegen of a Grid (ColumnDefinitions/RowDefinitions + Grid.Row/Grid.Column
    // attached properties -> .cell placement) yields the same tree as the hand-written grid builder. (The
    // runtime loader DEFERS attached properties — an M7 deferral — so the compile-time path handles MORE here;
    // this is a 2-way gate.)
    TEST(xaml_parity, grid_codegen_matches_handwritten)
    {
        auto hand = maui::ui::page(maui::ui::grid()
                                       .columns(maui::ui::star(), maui::ui::star())
                                       .rows(maui::ui::automatic(), maui::ui::automatic())
                                       .row_spacing(8)
                                       .column_spacing(8)
                                       .cell(0, 0, maui::ui::label("(0,0)"))
                                       .cell(0, 1, maui::ui::label("(0,1)"))
                                       .cell(1, 0, maui::ui::label("(1,0)"))
                                       .cell(1, 1, maui::ui::label("(1,1)")));
        auto generated = grid_page();

        const std::vector<grid_cell> hand_cells = describe_grid(hand.impl());
        const std::vector<grid_cell> gen_cells = describe_grid(generated.impl());

        EXPECT_EQ(hand_cells, gen_cells); // hand-written grid == compile-time codegen
        ASSERT_EQ(gen_cells.size(), 4U);
        EXPECT_EQ(gen_cells.front(), (grid_cell{0, 0, "(0,0)"}));
        EXPECT_EQ(gen_cells.back(), (grid_cell{1, 1, "(1,1)"}));
    }
} // namespace
