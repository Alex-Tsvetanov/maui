// Tests for indicator_view + indicator_view_handler — the cross-platform dot mirror: defaults, the
// ItemsSource → Count subscription, GetMaximumVisible (MaximumVisible / HideSingle collapse), the
// Position clamp + the FromHandler write-back, and the shape. Backend-agnostic (the dot mirror is the
// shared platform recipe on every backend; indicator_view_ios/apple tests assert the real control).
// Ported from IndicatorViewLayoutTests.cs + IndicatorViewExtensions.cs behavior + MauiPageControl.cs.
// §8: the items collection (publisher) is declared before the view/handler (subscriber).

#include <climits>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/indicator_view.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::indicator_shape;
    using maui::controls::indicator_view;
    using maui::core::indicator_view_handler;
    using maui::core::indicator_view_platform;
    using maui::core::max_visible_indicators;
    using maui::core::observable_collection;

    using string_collection = observable_collection<std::string>;

    // The standard rig: an attached handler so the dot mirror reflects the mapped surface.
    struct rig
    {
        indicator_view view;
        std::shared_ptr<indicator_view_handler> handler = std::make_shared<indicator_view_handler>();
        // The platform view exists only after set_handler connects the pair, so the initializer routes
        // through connect() (a static side-effecting setup helper — the collection_view_tests precedent;
        // a plain body assignment trips prefer-member-initializer, whose hoist would deref null).
        indicator_view_platform* platform = connect(view, handler);

        [[nodiscard]] static indicator_view_platform* connect(
            indicator_view& view_ref, const std::shared_ptr<indicator_view_handler>& handler_ref)
        {
            view_ref.set_handler(handler_ref);
            return handler_ref->typed_platform_view();
        }
    };

    // ---- defaults (IndicatorView property defaults) ----

    TEST(indicator_view, constructor_and_defaults)
    {
        const indicator_view view;
        EXPECT_EQ(view.count(), 0);
        EXPECT_EQ(view.position(), 0);
        EXPECT_EQ(view.maximum_visible(), INT_MAX);
        EXPECT_TRUE(view.hide_single());
        EXPECT_EQ(view.indicator_size(), 6.0);
        EXPECT_EQ(view.indicators_shape(), indicator_shape::circle);
        EXPECT_EQ(view.indicator_color(), maui::graphics::colors::light_grey);
        EXPECT_EQ(view.selected_indicator_color(), maui::graphics::colors::black);
        EXPECT_EQ(view.items_source(), nullptr);
    }

    // ---- ItemsSource drives Count (IndicatorView.ResetItemsSource / OnCollectionChanged) ----

    TEST(indicator_view, items_source_sets_count)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"item1", "item2"}); // publisher FIRST (§8)
        indicator_view view;
        view.set_items_source(items);
        EXPECT_EQ(view.count(), 2);
    }

    // A live mutation re-counts (the C# OnCollectionChanged path).
    TEST(indicator_view, items_source_live_mutation_recounts)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"a", "b"});
        indicator_view view;
        view.set_items_source(items);
        EXPECT_EQ(view.count(), 2);
        items->add("c");
        EXPECT_EQ(view.count(), 3);
        items->remove("a");
        EXPECT_EQ(view.count(), 2);
    }

    TEST(indicator_view, clearing_items_source_zeroes_count)
    {
        std::shared_ptr<string_collection> const items =
            std::make_shared<string_collection>(std::vector<std::string>{"a", "b", "c"});
        indicator_view view;
        view.set_items_source(items);
        EXPECT_EQ(view.count(), 3);
        view.clear_items_source();
        EXPECT_EQ(view.count(), 0);
    }

    // ---- GetMaximumVisible (IndicatorViewExtensions.GetMaximumVisible) ----

    TEST(indicator_view, max_visible_is_min_of_count_and_maximum_visible)
    {
        indicator_view view;
        view.set_count(10);
        view.set_maximum_visible(3);
        EXPECT_EQ(max_visible_indicators(view), 3);
        view.set_maximum_visible(INT_MAX);
        EXPECT_EQ(max_visible_indicators(view), 10);
    }

    TEST(indicator_view, max_visible_zero_when_count_zero)
    {
        const indicator_view view;
        EXPECT_EQ(max_visible_indicators(view), 0);
    }

    // HideSingle collapses a lone dot to 0 (default HideSingle == true).
    TEST(indicator_view, hide_single_collapses_a_lone_dot)
    {
        indicator_view view;
        view.set_count(1);
        EXPECT_TRUE(view.hide_single());
        EXPECT_EQ(max_visible_indicators(view), 0);
        view.set_hide_single(false);
        EXPECT_EQ(max_visible_indicators(view), 1);
    }

    // ---- the handler's dot mirror (Count → dot_count via UpdateIndicatorCount) ----

    TEST(indicator_view, handler_mirrors_dot_count_honoring_hide_single)
    {
        rig rig;
        ASSERT_NE(rig.platform, nullptr);
        rig.view.set_count(5);
        EXPECT_EQ(rig.platform->dot_count, 5);
        // HideSingle: a single dot collapses to 0 dots.
        rig.view.set_count(1);
        EXPECT_EQ(rig.platform->dot_count, 0);
        rig.view.set_hide_single(false);
        EXPECT_EQ(rig.platform->dot_count, 1);
    }

    // ---- Position clamp (MauiPageControl.GetCurrentPage: position >= maxVisible ? maxVisible-1 : position)

    TEST(indicator_view, handler_clamps_current_page_into_visible_range)
    {
        rig rig;
        ASSERT_NE(rig.platform, nullptr);
        rig.view.set_count(3);
        rig.view.set_position_manual(1);
        EXPECT_EQ(rig.platform->current_page, 1);
        // Position past the last dot clamps to the last index.
        rig.view.set_position_manual(9);
        EXPECT_EQ(rig.platform->current_page, 2); // dot_count 3 → last index 2
    }

    // ---- the FromHandler write-back (a native dot tap sets Position) ----

    TEST(indicator_view, set_position_from_handler_updates_position)
    {
        indicator_view view;
        view.set_position(2); // the inbound channel (FromHandler specificity)
        EXPECT_EQ(view.position(), 2);
    }

    // ---- shape mirror ----

    TEST(indicator_view, handler_mirrors_shape)
    {
        rig rig;
        ASSERT_NE(rig.platform, nullptr);
        EXPECT_EQ(rig.platform->shape, indicator_shape::circle);
        rig.view.set_indicators_shape(indicator_shape::square);
        EXPECT_EQ(rig.platform->shape, indicator_shape::square);
    }

    // ---- size + color mirror ----

    TEST(indicator_view, handler_mirrors_size_and_colors)
    {
        rig rig;
        ASSERT_NE(rig.platform, nullptr);
        rig.view.set_indicator_size(12.0);
        EXPECT_EQ(rig.platform->indicator_size, 12.0);
        rig.view.set_indicator_color(maui::graphics::colors::red);
        EXPECT_EQ(rig.platform->indicator_color, maui::graphics::colors::red);
        rig.view.set_selected_indicator_color(maui::graphics::colors::blue);
        EXPECT_EQ(rig.platform->selected_indicator_color, maui::graphics::colors::blue);
    }
} // namespace
