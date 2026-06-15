// Tests for the progress_bar control (maui::controls::progress_bar <= ProgressBar) and the headless
// handler seam. The control half ports src/Controls/tests/Core.UnitTests/ProgressBarTests.cs (the
// TestClamp oracle; TestProgressTo is NOT ported — ProgressTo needs the deferred animation subsystem,
// documented in progress_bar.hpp); the seam half follows the headless conventions (button_tests.cpp).
#include "maui/controls/progress_bar.hpp"

#include <memory>
#include <utility>

#include "maui/core/flow_direction.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::progress_bar;
    using maui::core::flow_direction;
    using maui::core::i_element;
    using maui::core::i_element_handler;
    using maui::core::progress_bar_handler;
    using maui::graphics::color;

    // A progress_bar whose logical parent the test can inject (the port's view::parent() weak-ref is not
    // wired by a layout yet, so the bar exposes a settable parent here to exercise the MatchParent →
    // parent-IView FlowDirection fallback of ProgressBarHandler.resolved_flow_direction). The parent is a
    // real control (any i_view with a flow_direction works), held alive so parent().lock() resolves.
    class parented_progress_bar : public progress_bar
    {
    public:
        void set_test_parent(std::shared_ptr<i_element> parent)
        {
            test_parent_ = std::move(parent);
        }
        [[nodiscard]] std::shared_ptr<i_element> parent() const override
        {
            return test_parent_;
        }

    private:
        std::shared_ptr<i_element> test_parent_;
    };

    // ---- the control in isolation (ProgressBarTests.cs) ----

    TEST(progress_bar, progress_defaults_to_zero)
    {
        const progress_bar control;
        EXPECT_EQ(control.progress(), 0.0);
    }

    TEST(progress_bar, progress_clamps_to_the_unit_range) // TestClamp
    {
        progress_bar control;
        control.set_progress(2);
        EXPECT_EQ(control.progress(), 1);

        control.set_progress(-1);
        EXPECT_EQ(control.progress(), 0);
    }

    TEST(progress_bar, in_range_progress_is_stored_unchanged)
    {
        progress_bar control;
        control.set_progress(0.45);
        EXPECT_EQ(control.progress(), 0.45);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(progress_bar_seam, attaching_handler_maps_initial_state)
    {
        progress_bar control;
        control.set_progress(0.6);
        control.set_progress_color(color(0.0F, 1.0F, 0.0F));
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->progress, 0.6);
        EXPECT_EQ(platform->progress_color, color(0.0F, 1.0F, 0.0F));
    }

    TEST(progress_bar_seam, setting_progress_updates_the_platform_clamped)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        control.set_progress(0.25);
        EXPECT_EQ(handler->typed_platform_view()->progress, 0.25);

        control.set_progress(7); // clamps to 1 before reaching the platform
        EXPECT_EQ(handler->typed_platform_view()->progress, 1.0);
    }

    TEST(progress_bar_seam, clearing_handler_disconnects)
    {
        progress_bar control;
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    // ---- FlowDirection mapper override (ProgressBarHandler.MapFlowDirection) ----

    TEST(progress_bar_seam, explicit_flow_direction_resolves_directly)
    {
        progress_bar control;
        control.set_flow_direction(flow_direction::right_to_left);
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::right_to_left);

        control.set_flow_direction(flow_direction::left_to_right);
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::left_to_right);
    }

    TEST(progress_bar_seam, match_parent_without_parent_stays_match_parent)
    {
        progress_bar control; // flow_direction defaults to match_parent; no parent
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::match_parent);
    }

    TEST(progress_bar_seam, match_parent_falls_back_to_parent_flow_direction)
    {
        auto parent = std::make_shared<progress_bar>();
        parent->set_flow_direction(flow_direction::right_to_left);

        parented_progress_bar control;
        control.set_test_parent(parent);
        control.set_flow_direction(flow_direction::match_parent);
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);

        // resolved_flow_direction sees MatchParent on the child → reads the parent IView's RTL.
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::right_to_left);
    }

    TEST(progress_bar_seam, explicit_child_direction_ignores_parent)
    {
        auto parent = std::make_shared<progress_bar>();
        parent->set_flow_direction(flow_direction::right_to_left);

        parented_progress_bar control;
        control.set_test_parent(parent);
        control.set_flow_direction(flow_direction::left_to_right); // explicit child wins over the parent
        auto handler = std::make_shared<progress_bar_handler>();
        control.set_handler(handler);
        EXPECT_EQ(handler->typed_platform_view()->resolved_flow_direction, flow_direction::left_to_right);
    }

    TEST(progress_bar_seam, handler_resolved_from_default_registry)
    {
        // progress_bar -> progress_bar_handler is self-registered in progress_bar.cpp.
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<progress_bar>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<progress_bar_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        progress_bar control;
        control.set_progress(0.5);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->progress, 0.5);
    }
} // namespace
