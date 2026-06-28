// Builder end-to-end test for the owning declarative tree (PUBLIC_API_DESIGN.md §3-C).
//
// Builds a counter-shaped tree (page -> vstack -> {label, button}) with the verbosity-free owning builder:
// ONE root view_ref owns the whole subtree (no reverse-ordered members), a .on_click token is parked in the
// tree, and a held label is observed through a weak_ref. Proves the tree mounts through the unchanged
// hosting path, handlers resolve, a simulated click runs the parked handler, and teardown is clean.

#include "maui/ui.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/size.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace
{
    namespace ui = maui::ui;
    using maui::controls::button;
    using maui::controls::label;

    // The counter, authored the new way: ONE owning root (root_) for the content tree, a weak_ref to the
    // one control mutated later, and only the top-level window/root ordered (window before root).
    class counter_app final : public ui::app
    {
    public:
        counter_app()
        {
            auto count_label = ui::label("Count: 0");
            count_label_ = count_label.weak();
            auto increment = ui::button("Increment");
            increment_ = increment.weak();

            root_ = ui::page(
                ui::vstack(std::move(count_label), std::move(increment).on_click([this] { bump(); })).spacing(12));

            window_.set_content(root_.impl()); // window hosts the page (root_ owns it)
            window_.set_title("Counter");
        }

        maui::controls::window& main_window() override
        {
            return window_;
        }

        void bump()
        {
            ++count_;
            if (auto l = count_label_.lock())
            {
                l->set_text("Count: " + std::to_string(count_));
            }
        }

        [[nodiscard]] int count() const
        {
            return count_;
        }
        ui::weak_ref<label>& label_observer()
        {
            return count_label_;
        }
        ui::weak_ref<button>& button_observer()
        {
            return increment_;
        }

    private:
        int count_ = 0;
        ui::weak_ref<label> count_label_; // non-owning observers
        ui::weak_ref<button> increment_;
        ui::view_ref<maui::controls::content_page> root_; // owns page -> vstack -> children
        // window_ references the page (set_content + its menu/toolbar trackers subscribe into the tree), so it
        // MUST be declared LAST -> destruct FIRST, while root_'s page is still alive (ASan-verified ordering).
        maui::controls::window window_;
    };

    TEST(ui_builder, builder_tree_mounts_and_click_bumps_label)
    {
        auto app = maui::hosting::maui_app::create_builder().use_maui_app<counter_app>().build();
        auto* a = app->application_as<counter_app>().get();
        ASSERT_NE(a, nullptr);

        maui::hosting::mount_window(*app, a->main_window());
        const maui::graphics::size arranged = maui::hosting::drive_layout(a->main_window(), 402.0, 874.0);
        EXPECT_GT(arranged.width, 0.0);
        EXPECT_GT(arranged.height, 0.0);

        // Handlers attached through the builder-owned tree.
        ASSERT_TRUE(a->label_observer().alive());
        ASSERT_TRUE(a->button_observer().alive());
        {
            auto l = a->label_observer().lock();
            ASSERT_TRUE(static_cast<bool>(l));
            EXPECT_NE(l->handler(), nullptr);
        }

        // Simulate a native click: the parked on_click token fires -> bump() -> the held label updates.
        {
            auto b = a->button_observer().lock();
            ASSERT_TRUE(static_cast<bool>(b));
            b->send_clicked();
        }
        EXPECT_EQ(a->count(), 1);
        {
            auto l = a->label_observer().lock();
            ASSERT_TRUE(static_cast<bool>(l));
            EXPECT_EQ(std::string(l->text()), "Count: 1");
        }
    }

    TEST(ui_builder, grid_builder_defines_and_places_cells)
    {
        auto grid = ui::grid()
                        .columns(ui::star(), ui::star())
                        .rows(ui::automatic(), ui::automatic())
                        .row_spacing(8)
                        .column_spacing(8)
                        .cell(0, 0, ui::label("top-left"))
                        .cell(1, 1, ui::label("bottom-right"));

        EXPECT_EQ(grid.impl().count(), 2);

        maui::core::i_view& first = grid.impl().at(0);
        maui::core::i_view& second = grid.impl().at(1);
        EXPECT_EQ(grid.impl().get_row(first), 0);
        EXPECT_EQ(grid.impl().get_column(first), 0);
        EXPECT_EQ(grid.impl().get_row(second), 1);
        EXPECT_EQ(grid.impl().get_column(second), 1);
        EXPECT_EQ(std::string(dynamic_cast<label&>(first).text()), "top-left");
        EXPECT_EQ(std::string(dynamic_cast<label&>(second).text()), "bottom-right");
    }
} // namespace
