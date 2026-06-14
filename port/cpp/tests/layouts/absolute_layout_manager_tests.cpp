// Ported from src/Core/tests/UnitTests/Layouts/AbsoluteLayoutManagerTests.cs — the behavioral oracle for
// the AbsoluteLayout algorithm: default (measured) bounds, absolute position/size, proportional position
// and/or size, padding, child measure constraints, min/max, and fill-on-arrange. A mock_absolute_layout
// (over the shared mock_view) plays the C# Substitute.For<IAbsoluteLayout>.
#include "maui/layouts/absolute_layout_manager.hpp"

#include <cstddef>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "maui/controls/view.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_absolute_layout.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    // The shared child mock from the layout test helpers (records measure/arrange + returns a fixed size).
    using maui::layouts::testing::mock_view;

    // A configurable IAbsoluteLayout over a list of child views (non-owning). Mirrors the C#
    // Substitute.For<IAbsoluteLayout>(): per-child bounds + flags, plus the layout's own size requests.
    class mock_absolute_layout : public maui::controls::view<maui::core::i_absolute_layout>
    {
    public:
        struct info
        {
            maui::graphics::rect bounds{0, 0, -1, -1}; // AutoSize default
            maui::layouts::absolute_layout_flags flags = maui::layouts::absolute_layout_flags::none;
        };

        // i_container
        [[nodiscard]] int count() const override
        {
            return static_cast<int>(children.size());
        }
        [[nodiscard]] maui::core::i_view& at(int index) const override
        {
            return *children[static_cast<std::size_t>(index)];
        }
        void add(maui::core::i_view& child) override
        {
            children.push_back(&child);
            child_info.emplace_back();
        }
        void insert(int index, maui::core::i_view& child) override
        {
            children.insert(children.begin() + index, &child);
            child_info.insert(child_info.begin() + index, info{});
        }
        void remove_at(int index) override
        {
            children.erase(children.begin() + index);
            child_info.erase(child_info.begin() + index);
        }
        void clear() override
        {
            children.clear();
            child_info.clear();
        }
        [[nodiscard]] int index_of(const maui::core::i_view& child) const override
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &child)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // i_padding / i_layout
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_value;
        }
        [[nodiscard]] bool clips_to_bounds() const override
        {
            return false;
        }

        // i_absolute_layout
        [[nodiscard]] maui::graphics::rect get_layout_bounds(const maui::core::i_view& view) const override
        {
            return info_for(view).bounds;
        }
        [[nodiscard]] maui::layouts::absolute_layout_flags get_layout_flags(
            const maui::core::i_view& view) const override
        {
            return info_for(view).flags;
        }

        void set_bounds(const maui::core::i_view& view, maui::graphics::rect bounds)
        {
            mutable_info_for(view).bounds = bounds;
        }
        void set_flags(const maui::core::i_view& view, maui::layouts::absolute_layout_flags flags)
        {
            mutable_info_for(view).flags = flags;
        }

        // Configurable layout dimensions (override view<>'s frame-derived defaults).
        [[nodiscard]] double width() const override
        {
            return width_value;
        }
        [[nodiscard]] double height() const override
        {
            return height_value;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return min_width_value;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return max_width_value;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return min_height_value;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return max_height_value;
        }

        std::vector<maui::core::i_view*> children;
        std::vector<info> child_info; // parallel to children
        maui::core::thickness padding_value;
        double width_value = maui::core::dimension::unset;
        double height_value = maui::core::dimension::unset;
        double min_width_value = maui::core::dimension::minimum;
        double max_width_value = maui::core::dimension::maximum;
        double min_height_value = maui::core::dimension::minimum;
        double max_height_value = maui::core::dimension::maximum;

    private:
        [[nodiscard]] std::size_t index_of_view(const maui::core::i_view& view) const
        {
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] == &view)
                {
                    return i;
                }
            }
            return children.size();
        }
        [[nodiscard]] const info& info_for(const maui::core::i_view& view) const
        {
            const std::size_t i = index_of_view(view);
            if (i < children.size())
            {
                return child_info[i];
            }
            static const info fallback;
            return fallback;
        }
        [[nodiscard]] info& mutable_info_for(const maui::core::i_view& view)
        {
            return child_info[index_of_view(view)];
        }
    };

    // Owns the layout's children, mirroring the C# CreateTestView / SubstituteChildren helpers.
    class absolute_fixture
    {
    public:
        mock_absolute_layout layout;

        // C# CreateTestView(Size): a view whose DesiredSize is the given size (no explicit measure needed).
        mock_view& add_view(maui::graphics::size desired = {})
        {
            auto view = std::make_unique<mock_view>();
            view->configure(desired);
            mock_view& reference = *view;
            layout.add(reference);
            owned_.push_back(std::move(view));
            return reference;
        }

    private:
        std::vector<std::unique_ptr<mock_view>> owned_;
    };

    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::absolute_layout_flags;
    using maui::layouts::absolute_layout_manager;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // measure(w, h) then arrange at (left, top, measured).
    size measure_and_arrange(absolute_fixture& fixture, double width = inf, double height = inf, double left = 0,
                             double top = 0)
    {
        absolute_layout_manager manager(fixture.layout);
        const size measured = manager.measure(width, height);
        manager.arrange_children(rect(left, top, measured.width, measured.height));
        return measured;
    }

    TEST(absolute_layout_manager, default_layout_bounds_uses_default_measure)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view({50, 75});
        fixture.layout.set_bounds(child, rect(0, 0, -1, -1)); // the AutoSize default

        const size measured = measure_and_arrange(fixture, inf, inf);

        EXPECT_EQ(measured, size(50, 75));
        EXPECT_EQ(child.last_arrange, rect(0, 0, 50, 75));
    }

    TEST(absolute_layout_manager, absolute_position_and_size)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        const rect expected(10, 15, 100, 100);
        fixture.layout.set_bounds(child, expected);

        const size measured = measure_and_arrange(fixture, inf, inf);

        EXPECT_EQ(measured, size(expected.left() + expected.width, expected.top() + expected.height));
        EXPECT_EQ(child.last_arrange, expected);
    }

    TEST(absolute_layout_manager, absolute_layout_respects_bounds)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        const rect child_bounds(10, 15, 100, 100);
        fixture.layout.set_bounds(child, child_bounds);

        const size measured = measure_and_arrange(fixture, inf, inf, /*left=*/10, /*top=*/10);

        EXPECT_EQ(measured, size(child_bounds.left() + child_bounds.width, child_bounds.top() + child_bounds.height));
        EXPECT_EQ(child.last_arrange,
                  rect(child_bounds.left() + 10, child_bounds.top() + 10, child_bounds.width, child_bounds.height));
    }

    TEST(absolute_layout_manager, absolute_position_relative_size)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(10, 20, 0.4, 0.5));
        fixture.layout.set_flags(child, absolute_layout_flags::size_proportional);

        absolute_layout_manager manager(fixture.layout);
        const size measured = manager.measure(100, 100);
        manager.arrange_children(rect(0, 0, 100, 100));

        EXPECT_EQ(measured, size(10 + 40, 20 + 50));
        EXPECT_EQ(child.last_arrange, rect(10, 20, 40, 50));
    }

    struct relative_position_absolute_size_case
    {
        double width;
        double height;
        double prop_x;
        double prop_y;
    };

    class relative_position_absolute_size : public ::testing::TestWithParam<relative_position_absolute_size_case>
    {
    };

    TEST_P(relative_position_absolute_size, positions_proportionally)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(p.prop_x, p.prop_y, p.width, p.height));
        fixture.layout.set_flags(child, absolute_layout_flags::position_proportional);

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(100, 100);
        manager.arrange_children(rect(0, 0, 100, 100));

        const double expected_x = (100 - p.width) * p.prop_x;
        const double expected_y = (100 - p.height) * p.prop_y;
        EXPECT_EQ(child.last_arrange, rect(expected_x, expected_y, p.width, p.height));
    }

    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, relative_position_absolute_size,
                             ::testing::Values(relative_position_absolute_size_case{30, 40, 0.2, 0.3},
                                               relative_position_absolute_size_case{35, 45, 0.5, 0.5},
                                               relative_position_absolute_size_case{35, 45, 0, 0},
                                               relative_position_absolute_size_case{35, 45, 1, 1}));

    struct relative_position_relative_size_case
    {
        double prop_x;
        double prop_y;
        double prop_height;
        double prop_width;
    };

    class relative_position_relative_size : public ::testing::TestWithParam<relative_position_relative_size_case>
    {
    };

    TEST_P(relative_position_relative_size, positions_and_sizes_proportionally)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(p.prop_x, p.prop_y, p.prop_width, p.prop_height));
        fixture.layout.set_flags(child, absolute_layout_flags::all);

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(100, 100);
        manager.arrange_children(rect(0, 0, 100, 100));

        const double expected_width = 100 * p.prop_width;
        const double expected_height = 100 * p.prop_height;
        const double expected_x = (100 - expected_width) * p.prop_x;
        const double expected_y = (100 - expected_height) * p.prop_y;
        EXPECT_EQ(child.last_arrange, rect(expected_x, expected_y, expected_width, expected_height));
    }

    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, relative_position_relative_size,
                             ::testing::Values(relative_position_relative_size_case{0.0, 0.0, 0.0, 0.0},
                                               relative_position_relative_size_case{0.2, 0.2, 0.2, 0.2},
                                               relative_position_relative_size_case{0.5, 0.5, 0.5, 0.5},
                                               relative_position_relative_size_case{1.0, 1.0, 1.0, 1.0}));

    struct padding_case
    {
        double left;
        double top;
        double right;
        double bottom;
        double expected_x;
        double expected_y;
    };

    class relative_position_respects_padding : public ::testing::TestWithParam<padding_case>
    {
    };

    TEST_P(relative_position_respects_padding, offsets_by_padding)
    {
        const auto& p = GetParam();
        constexpr double width = 20;
        constexpr double height = 20;
        constexpr double prop = 0.5;

        absolute_fixture fixture;
        fixture.layout.padding_value = thickness(p.left, p.top, p.right, p.bottom);
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(prop, prop, width, height));
        fixture.layout.set_flags(child, absolute_layout_flags::position_proportional);

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(100, 100);
        manager.arrange_children(rect(0, 0, 100, 100));

        EXPECT_EQ(child.last_arrange, rect(p.expected_x, p.expected_y, width, height));
    }

    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, relative_position_respects_padding,
                             ::testing::Values(padding_case{0, 0, 0, 0, 40, 40}, padding_case{5, 5, 5, 5, 40, 40},
                                               padding_case{10, 10, 0, 0, 45, 45}));

    struct minmax_case
    {
        double limit;
        double view_dimension;
        double expected;
    };

    class measure_respects_max_height : public ::testing::TestWithParam<minmax_case>
    {
    };
    TEST_P(measure_respects_max_height, clamps)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, p.view_dimension));
        fixture.layout.max_height_value = p.limit;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).height, p.expected);
    }
    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, measure_respects_max_height,
                             ::testing::Values(minmax_case{50, 100, 50}, minmax_case{100, 100, 100},
                                               minmax_case{100, 50, 50}, minmax_case{0, 50, 0}));

    class measure_respects_max_width : public ::testing::TestWithParam<minmax_case>
    {
    };
    TEST_P(measure_respects_max_width, clamps)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, p.view_dimension, 100));
        fixture.layout.max_width_value = p.limit;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).width, p.expected);
    }
    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, measure_respects_max_width,
                             ::testing::Values(minmax_case{50, 100, 50}, minmax_case{100, 100, 100},
                                               minmax_case{100, 50, 50}, minmax_case{0, 50, 0}));

    class measure_respects_min_height : public ::testing::TestWithParam<minmax_case>
    {
    };
    TEST_P(measure_respects_min_height, floors)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, p.view_dimension));
        fixture.layout.min_height_value = p.limit;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).height, p.expected);
    }
    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, measure_respects_min_height,
                             ::testing::Values(minmax_case{50, 10, 50}, minmax_case{100, 100, 100},
                                               minmax_case{10, 50, 50}));

    class measure_respects_min_width : public ::testing::TestWithParam<minmax_case>
    {
    };
    TEST_P(measure_respects_min_width, floors)
    {
        const auto& p = GetParam();
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, p.view_dimension, 100));
        fixture.layout.min_width_value = p.limit;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).width, p.expected);
    }
    INSTANTIATE_TEST_SUITE_P(absolute_layout_manager, measure_respects_min_width,
                             ::testing::Values(minmax_case{50, 10, 50}, minmax_case{100, 100, 100},
                                               minmax_case{10, 50, 50}));

    TEST(absolute_layout_manager, max_width_dominates_width)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, 100));
        fixture.layout.width_value = 75;
        fixture.layout.max_width_value = 50;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).width, 50); // max beats the explicit value
    }

    TEST(absolute_layout_manager, min_width_dominates_max_width)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, 100));
        fixture.layout.min_width_value = 75;
        fixture.layout.max_width_value = 50;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).width, 75); // min beats max
    }

    TEST(absolute_layout_manager, max_height_dominates_height)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, 100));
        fixture.layout.height_value = 75;
        fixture.layout.max_height_value = 50;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).height, 50);
    }

    TEST(absolute_layout_manager, min_height_dominates_max_height)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, 100));
        fixture.layout.min_height_value = 75;
        fixture.layout.max_height_value = 50;

        absolute_layout_manager manager(fixture.layout);
        EXPECT_EQ(manager.measure(inf, inf).height, 75);
    }

    TEST(absolute_layout_manager, arrange_accounts_for_fill)
    {
        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, 100, 100));

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(inf, inf);

        const size actual = manager.arrange_children(rect(0, 0, 1000, 1000));
        EXPECT_EQ(actual.width, 1000);
        EXPECT_EQ(actual.height, 1000);
    }

    TEST(absolute_layout_manager, child_measure_respects_absolute_bounds)
    {
        constexpr double expected_width = 115;
        constexpr double expected_height = 230;

        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, expected_width, expected_height));

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(inf, inf);

        EXPECT_EQ(child.last_measure_width, expected_width);
        EXPECT_EQ(child.last_measure_height, expected_height);
    }

    TEST(absolute_layout_manager, child_measure_respects_proportional_bounds)
    {
        constexpr double prop_width = 0.5;
        constexpr double prop_height = 0.6;
        constexpr double width_constraint = 200;
        constexpr double height_constraint = 200;

        absolute_fixture fixture;
        mock_view& child = fixture.add_view();
        fixture.layout.set_bounds(child, rect(0, 0, prop_width, prop_height));
        fixture.layout.set_flags(child, absolute_layout_flags::size_proportional);

        absolute_layout_manager manager(fixture.layout);
        std::ignore = manager.measure(width_constraint, height_constraint);

        EXPECT_EQ(child.last_measure_width, prop_width * width_constraint);
        EXPECT_EQ(child.last_measure_height, prop_height * height_constraint);
    }
} // namespace
