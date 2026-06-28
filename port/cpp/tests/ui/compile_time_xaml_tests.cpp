// Compile-time XAML (PUBLIC_API_DESIGN.md §6): the COMPILER embeds raw markup (#embed) into the TU and
// build_page<VM, Xaml>() hydrates it — no build-system codegen step, so this path is the one that cross-
// compiles to the iOS app bundle. These tests prove the member-free story end to end:
//   (1) build_page hydrates the embedded markup INTO a page_impl<VM> (structure + literal props), and
//   (2) a view-model of plain maui::property/maui::command cells drives the view via code-behind found by
//       x:Name — no per-widget member variables, no ordering constraints (the verbose-tree complaint that
//       motivated this work).

#include "maui/command.hpp"
#include "maui/fixed_string.hpp"
#include "maui/property.hpp"
#include "maui/xaml_build.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace
{
    namespace controls = maui::controls;

    // A member-free view-model: only bindable cells + a command. No widget instances, no fixed ordering.
    struct counter_view_model
    {
        maui::property<int> Count{0};
        maui::command Increment;
        counter_view_model() : Increment{[this] { Count(Count() + 1); }}
        {
        }
    };

    // The raw markup, embedded by the compiler (resolved relative to this source file). The two-line embed
    // is the canonical form (a #embed directive cannot live inside a macro body — see fixed_string.hpp).
    constexpr unsigned char counter_bytes[] = {
#embed "fixtures/counter.xaml"
    };
    constexpr maui::fixed_string counter_xaml{counter_bytes};

    TEST(compile_time_xaml, hydrates_structure_from_embedded_markup)
    {
        auto page = maui::build_page<counter_view_model, counter_xaml>();
        ASSERT_NE(page, nullptr);

        auto* stack = dynamic_cast<controls::vertical_stack_layout*>(page->content());
        ASSERT_NE(stack, nullptr);
        EXPECT_EQ(stack->spacing(), 12.0);
        ASSERT_EQ(stack->count(), 2);
        EXPECT_EQ(std::string(dynamic_cast<controls::label&>(stack->at(0)).text()), "Count: 0");
        EXPECT_EQ(std::string(dynamic_cast<controls::button&>(stack->at(1)).text()), "Increment");
    }

    TEST(compile_time_xaml, find_by_name_and_member_free_code_behind)
    {
        auto page = maui::build_page<counter_view_model, counter_xaml>();
        page->bind_to(std::make_unique<counter_view_model>());
        auto* view_model = page->view_model();
        ASSERT_NE(view_model, nullptr);

        // x:Name lookup — the only handles code-behind needs (no per-widget members on the page).
        auto label = page->find<controls::label>("CountLabel");
        auto button = page->find<controls::button>("IncrementButton");
        ASSERT_NE(label, nullptr);
        ASSERT_NE(button, nullptr);

        // Code-behind wiring: VM property -> label text, button click -> command (typed, compile-checked).
        // The tokens are lifetime guards (their destructors disconnect) — held, not otherwise read.
        [[maybe_unused]] auto label_token =
            view_model->Count.changed.connect([weak = std::weak_ptr<controls::label>(label)](int /*old*/, int now) {
                if (auto live = weak.lock())
                {
                    live->set_text("Count: " + std::to_string(now));
                }
            });
        auto click_token =
            maui::core::connect_scoped(button->clicked, [view_model] { view_model->Increment.execute(); });

        EXPECT_EQ(std::string(label->text()), "Count: 0");
        button->send_clicked();
        EXPECT_EQ(view_model->Count(), 1);
        EXPECT_EQ(std::string(label->text()), "Count: 1");
        button->send_clicked();
        EXPECT_EQ(view_model->Count(), 2);
        EXPECT_EQ(std::string(label->text()), "Count: 2");
    }

    // A bindable_object view-model whose property NAME matches the markup {Binding} path.
    class greeting_view_model : public maui::core::bindable_object
    {
    public:
        maui::core::observable<std::string> Message{*this, "Message", "World"};
    };

    constexpr unsigned char data_binding_bytes[] = {
#embed "fixtures/data_binding.xaml"
    };
    constexpr maui::fixed_string data_binding_xaml{data_binding_bytes};

    // The "fully markup-bound" path: the markup carries {Binding Message} (no code-behind binding). build_page
    // hydrates it; bind_to sets the page's BindingContext to the VM, which makes the loader-attached bindings
    // resolve against the registered "Message" property — live, with no reflection.
    TEST(compile_time_xaml, markup_binding_resolves_against_binding_context)
    {
        auto page = maui::build_page<greeting_view_model, data_binding_xaml>();
        page->bind_to(std::make_unique<greeting_view_model>());
        auto* view_model = page->view_model();
        ASSERT_NE(view_model, nullptr);

        auto greeting = page->find<controls::label>("greeting");
        ASSERT_NE(greeting, nullptr);

        // {Binding Message} resolved against the BindingContext default ("World").
        EXPECT_EQ(std::string(greeting->text()), "World");
        // Updating the VM property flows to the bound label (one-way markup binding, no code-behind).
        view_model->Message.set("Alex");
        EXPECT_EQ(std::string(greeting->text()), "Alex");
    }

    constexpr unsigned char grid_bytes[] = {
#embed "fixtures/grid.xaml"
    };
    constexpr maui::fixed_string grid_xaml{grid_bytes};

    // Grid attached properties (Grid.Row / Grid.Column) — set on the child in markup, stored by the parent
    // grid. The runtime loader used to DEFER these (a load failure); build_page now places the children.
    TEST(compile_time_xaml, grid_attached_properties_place_children)
    {
        auto page = maui::build_page<maui::no_view_model, grid_xaml>();
        auto* grid = dynamic_cast<controls::grid*>(page->content());
        ASSERT_NE(grid, nullptr);
        ASSERT_EQ(grid->count(), 4);
        EXPECT_EQ(grid->row_spacing(), 8.0);
        // ColumnDefinitions="*,*" / RowDefinitions="Auto,Auto" parsed into the grid's definition vectors.
        ASSERT_EQ(grid->column_definitions().size(), 2U);
        ASSERT_EQ(grid->row_definitions().size(), 2U);
        EXPECT_TRUE(grid->column_definitions()[0].width().is_star());
        EXPECT_TRUE(grid->row_definitions()[0].height().is_auto());

        const auto cell = [&](int i) {
            maui::core::i_view& view = grid->at(i);
            return std::pair{grid->get_row(view), grid->get_column(view)};
        };
        EXPECT_EQ(cell(0), (std::pair{0, 0}));
        EXPECT_EQ(cell(1), (std::pair{0, 1}));
        EXPECT_EQ(cell(2), (std::pair{1, 0}));
        EXPECT_EQ(cell(3), (std::pair{1, 1}));
    }
} // namespace
