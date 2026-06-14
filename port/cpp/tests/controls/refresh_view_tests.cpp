// Tests for the refresh_view control + its headless handler seam — ported from RefreshViewTests.cs
// (src/Controls/tests/Core.UnitTests). The port's command channel is the W1-11 ICommand collapse
// (set_command(action, can_execute) + change_can_execute()), so the C# Command/CanExecute tests map onto
// that surface; everything else (the IsRefreshing coerce/changed, IsRefreshEnabled coerce/changed, the
// IsEnabled→stop-refresh path, the Refreshing event) ports 1:1. The seam mirrors IsRefreshing /
// IsRefreshEnabled / the spinner color / the hosted content, and request_refresh() is the native pull
// stand-in (MauiRefreshViewProxy.OnRefresh → IsRefreshing=true write-back).
#include "maui/controls/refresh_view.hpp"

#include <any>
#include <memory>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/graphics/colors.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::refresh_view;
    using maui::core::i_element_handler;
    using maui::core::refresh_view_handler;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation ----

    TEST(refresh_view, starts_enabled) // C# StartsEnabled
    {
        refresh_view view;
        EXPECT_TRUE(view.is_enabled());
    }

    TEST(refresh_view, is_refresh_enabled_defaults_to_true) // C# IsRefreshEnabledDefaultsToTrue
    {
        refresh_view view;
        EXPECT_TRUE(view.is_refresh_enabled());
    }

    TEST(refresh_view, can_execute_false_disables_refreshing) // C# CanExecuteDisablesRefreshing
    {
        refresh_view view;
        view.set_command([] {}, [] { return false; });
        EXPECT_FALSE(view.is_refresh_enabled());
    }

    TEST(refresh_view, can_execute_true_enables_refreshing) // C# CanExecuteEnablesRefreshing
    {
        refresh_view view;
        view.set_command([] {}, [] { return true; });
        EXPECT_TRUE(view.is_refresh_enabled());
    }

    TEST(refresh_view, is_refreshing_still_toggles_true_when_can_execute_toggled_during_execute)
    {
        // C# IsRefreshingStillTogglesTrueWhenCanExecuteToggledDuringExecute: a command whose CanExecute is
        // !IsRefreshing, and which flips its own CanExecute when run. IsRefreshing must still land true.
        refresh_view view;
        view.set_command([&view] { view.change_can_execute(); }, [&view] { return !view.is_refreshing(); });
        view.set_is_refreshing(true);
        EXPECT_TRUE(view.is_refreshing());
    }

    TEST(refresh_view, is_refresh_enabled_coerces_can_execute) // C# IsRefreshEnabledShouldCoerceCanExecute
    {
        refresh_view view;
        view.set_is_refresh_enabled(false);
        view.set_command([] {});
        EXPECT_FALSE(view.is_refresh_enabled());
    }

    TEST(refresh_view, can_execute_changes_is_refresh_enabled) // C# CanExecuteChangesIsRefreshEnabled
    {
        refresh_view view;
        bool can_execute = true;
        view.set_command([] {}, [&can_execute] { return can_execute; });

        can_execute = false;
        view.change_can_execute();
        EXPECT_FALSE(view.is_refresh_enabled());

        can_execute = true;
        view.change_can_execute();
        EXPECT_TRUE(view.is_refresh_enabled());
    }

    TEST(refresh_view, command_parameter_changes_is_refresh_enabled) // C# CommandPropertyChangesIsRefreshEnabled
    {
        refresh_view view;
        // A parameterized command: CanExecute(p) == (p is a non-null true bool).
        const auto can_execute = [](const std::any& p) {
            const auto* value = std::any_cast<bool>(&p);
            return value != nullptr && *value;
        };
        view.set_command([](const std::any&) {}, can_execute, std::any(true));
        EXPECT_TRUE(view.is_refresh_enabled());

        view.set_command_parameter(std::any(false));
        EXPECT_FALSE(view.is_refresh_enabled());
        view.set_command_parameter(std::any(true));
        EXPECT_TRUE(view.is_refresh_enabled());
    }

    TEST(refresh_view, removed_command_enables_refresh_view) // C# RemovedCommandEnablesRefreshView
    {
        refresh_view view;
        const auto cant = [] { return false; };
        view.set_command([] {}, cant);
        EXPECT_FALSE(view.is_refresh_enabled());
        view.set_command({}); // clear the command
        EXPECT_TRUE(view.is_refresh_enabled());
        view.set_command([] {}, cant);
        EXPECT_FALSE(view.is_refresh_enabled());
    }

    TEST(refresh_view, is_refreshing_stays_false_with_disabled_command) // C# IsRefreshingStaysFalseWithDisabledCommand
    {
        refresh_view view;
        view.set_command([] {}, [] { return false; });
        view.set_is_refreshing(true);
        EXPECT_FALSE(view.is_refreshing());
    }

    TEST(refresh_view, is_refreshing_settable_to_true) // C# IsRefreshingSettableToTrue
    {
        refresh_view view;
        EXPECT_FALSE(view.is_refreshing());
        view.set_is_refreshing(true);
        EXPECT_TRUE(view.is_refreshing());
    }

    TEST(refresh_view, is_refreshing_stays_false_with_disabled_view) // C# IsRefreshingStaysFalseWithDisabledRefreshView
    {
        refresh_view view;
        view.set_is_enabled(false);
        view.set_is_refreshing(true);
        EXPECT_FALSE(view.is_refreshing());
    }

    TEST(refresh_view,
         is_refreshing_toggles_false_when_is_enabled_set_false) // C# IsRefreshingTogglesFalseWhenIsEnabledSetToFalse
    {
        refresh_view view;
        view.set_is_refreshing(true);
        view.set_is_enabled(false);
        EXPECT_FALSE(view.is_refreshing());
    }

    TEST(refresh_view, is_refreshing_event_fires) // C# IsRefreshingEventFires
    {
        refresh_view view;
        bool fired = false;
        view.refreshing.connect([&fired] { fired = true; });
        EXPECT_FALSE(fired);
        view.set_is_refreshing(true);
        EXPECT_TRUE(fired);
    }

    TEST(refresh_view, is_refresh_enabled_can_be_set_to_false) // C# IsRefreshEnabledCanBeSetToFalse
    {
        refresh_view view;
        view.set_is_refresh_enabled(false);
        EXPECT_FALSE(view.is_refresh_enabled());
    }

    TEST(refresh_view,
         is_refresh_enabled_prevents_is_refreshing_true) // C# IsRefreshEnabledPreventsIsRefreshingFromBeingSetToTrue
    {
        refresh_view view;
        view.set_is_refresh_enabled(false);
        view.set_is_refreshing(true);
        EXPECT_FALSE(view.is_refreshing());
    }

    TEST(refresh_view, setting_is_refresh_enabled_false_while_refreshing_stops_refresh)
    {
        // C# SettingIsRefreshEnabledToFalseWhileRefreshingStopsRefresh.
        refresh_view view;
        view.set_is_refreshing(true);
        EXPECT_TRUE(view.is_refreshing());
        view.set_is_refresh_enabled(false);
        EXPECT_FALSE(view.is_refreshing());
    }

    struct refresh_behavior_case
    {
        bool is_enabled;
        bool is_refresh_enabled;
        bool expected_refreshing;
    };

    class refresh_behavior_param : public ::testing::TestWithParam<refresh_behavior_case>
    {
    };

    TEST_P(refresh_behavior_param, depends_on_is_enabled_and_is_refresh_enabled)
    {
        // C# RefreshBehaviorDependsOnIsEnabledAndIsRefreshEnabled.
        const refresh_behavior_case c = GetParam();
        refresh_view view;
        view.set_is_enabled(c.is_enabled);
        view.set_is_refresh_enabled(c.is_refresh_enabled);
        view.set_is_refreshing(true);
        EXPECT_EQ(view.is_refreshing(), c.expected_refreshing);
    }

    INSTANTIATE_TEST_SUITE_P(refresh_view, refresh_behavior_param,
                             ::testing::Values(refresh_behavior_case{true, true, true},
                                               refresh_behavior_case{false, true, false},
                                               refresh_behavior_case{true, false, false},
                                               refresh_behavior_case{false, false, false}));

    TEST(refresh_view, is_refresh_enabled_works_with_command) // C# IsRefreshEnabledWorksWithCommand
    {
        refresh_view view;
        bool executed = false;
        view.set_command([&executed] { executed = true; });

        view.set_is_refresh_enabled(true);
        view.set_is_refreshing(true);
        EXPECT_TRUE(executed);

        executed = false;
        view.set_is_refreshing(false);

        view.set_is_refresh_enabled(false);
        view.set_is_refreshing(true);
        EXPECT_FALSE(view.is_refreshing());
        EXPECT_FALSE(executed);
    }

    TEST(refresh_view, is_refresh_enabled_respects_command_can_execute) // C# IsRefreshEnabledRespectsCommandCanExecute
    {
        refresh_view view;
        bool can_execute = true;
        view.set_command([] {}, [&can_execute] { return can_execute; });
        EXPECT_TRUE(view.is_refresh_enabled());

        can_execute = false;
        view.change_can_execute();
        EXPECT_FALSE(view.is_refresh_enabled());

        can_execute = true;
        view.change_can_execute();
        EXPECT_TRUE(view.is_refresh_enabled());
    }

    TEST(refresh_view, can_execute_false_blocks_refresh) // C# IsRefreshEnabledWithCommandCanExecuteFalseBlocksRefresh
    {
        refresh_view view;
        bool executed = false;
        view.set_command([&executed] { executed = true; }, [] { return false; });

        view.set_is_refresh_enabled(true);
        EXPECT_FALSE(view.is_refresh_enabled()); // coerced to false by CanExecute

        view.set_is_refreshing(true);
        EXPECT_FALSE(view.is_refreshing());
        EXPECT_FALSE(executed);
    }

    TEST(refresh_view, can_execute_change_does_not_stop_active_refresh)
    {
        // C# CommandCanExecuteChangeClearsIsRefreshingWhenBecomesFalse: while already refreshing, a
        // CanExecute change does NOT stop the refresh (the CanExecuteChanged early-out).
        refresh_view view;
        bool can_execute = true;
        bool executed = false;
        view.set_command([&executed] { executed = true; }, [&can_execute] { return can_execute; });

        view.set_is_refreshing(true);
        EXPECT_TRUE(view.is_refreshing());
        EXPECT_TRUE(executed);

        can_execute = false;
        view.change_can_execute();
        EXPECT_TRUE(view.is_refreshing());
    }

    TEST(refresh_view, refresh_color_round_trips)
    {
        refresh_view view;
        EXPECT_FALSE(view.has_refresh_color());
        EXPECT_EQ(view.refresh_color(), nullptr); // IRefreshView.RefreshColor null when unset
        view.set_refresh_color(maui::graphics::colors::blue);
        EXPECT_TRUE(view.has_refresh_color());
        ASSERT_NE(view.refresh_color(), nullptr);
        EXPECT_EQ(view.refresh_color()->background_color().to_uint(), maui::graphics::colors::blue.to_uint());
    }

    // ---- the headless handler seam ----

    TEST(refresh_view_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<refresh_view>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<refresh_view_handler*>(handler.get()), nullptr);
    }

    TEST(refresh_view_seam, content_is_refreshing_and_color_mirror_onto_the_platform)
    {
        mock_view content;
        refresh_view view;
        view.set_content(content);
        view.set_refresh_color(maui::graphics::colors::green);

        auto handler = std::make_shared<refresh_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        EXPECT_EQ(platform->hosted_content, &content);
        EXPECT_TRUE(platform->has_refresh_color);
        EXPECT_EQ(platform->refresh_color_argb, maui::graphics::colors::green.to_uint());
        EXPECT_FALSE(platform->refreshing);

        view.set_is_refreshing(true); // a runtime change flows through the mapper
        EXPECT_TRUE(platform->refreshing);
    }

    TEST(refresh_view_seam, request_refresh_writes_is_refreshing_back_and_runs_the_command)
    {
        // The native pull stand-in: handler.request_refresh() → IsRefreshing=true → Refreshing + command.
        refresh_view view;
        int refreshed = 0;
        view.refreshing.connect([&refreshed] { ++refreshed; });
        bool executed = false;
        view.set_command([&executed] { executed = true; });

        auto handler = std::make_shared<refresh_view_handler>();
        view.set_handler(handler);

        handler->request_refresh();
        EXPECT_TRUE(view.is_refreshing());
        EXPECT_EQ(refreshed, 1);
        EXPECT_TRUE(executed);
    }
} // namespace
