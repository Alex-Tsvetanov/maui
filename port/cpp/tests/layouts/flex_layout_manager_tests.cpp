// Ported from the FlexLayout behavioral oracle: the modern src/Controls/tests/Core.UnitTests/Layouts/
// FlexLayoutTests.cs (unconstrained measure, padding, direction, the #34464 grow/free-space fix, the
// #31109 arrange-only WidthRequest path) + the algorithm cases from FlexLayoutMarginTests.cs /
// FlexOrderTests.cs (margins, justify, ordering) which exercise the shared Flex engine. The tests drive
// the real flex_layout control (which internally uses the flex_layout_manager + Flex engine) over
// flex_child views (a mock_view with configurable margin + size requests).
#include "maui/controls/flex_layout.hpp"

#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/flex_enums.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace
{
    using maui::controls::flex_layout;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::flex_align_items;
    using maui::layouts::flex_direction;
    using maui::layouts::flex_justify;
    using maui::layouts::testing::mock_view;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // A flex child: a mock_view that measures to a fixed size and carries a configurable margin (the base
    // mock_view returns no margin), so the margin-distribution cases can be exercised.
    class flex_child : public mock_view
    {
    public:
        [[nodiscard]] maui::core::thickness margin() const override
        {
            return margin_value;
        }
        maui::core::thickness margin_value;
    };

    // Owns the flex_layout's children, mirroring the C# layout + Children setup.
    class flex_fixture
    {
    public:
        flex_layout layout;

        flex_child& add_child(size measured, thickness margin = {})
        {
            auto child = std::make_unique<flex_child>();
            child->configure(measured);
            child->margin_value = margin;
            flex_child& reference = *child;
            layout.add(reference);
            owned_.push_back(std::move(child));
            return reference;
        }

    private:
        std::vector<std::unique_ptr<flex_child>> owned_;
    };

    // measure(w, h) then arrange at (0, 0, w, h). Returns the measured size.
    size measure_and_arrange(flex_fixture& fixture, double width, double height)
    {
        const size measured = fixture.layout.measure(width, height);
        fixture.layout.arrange(rect(0, 0, width, height));
        return measured;
    }

    // ---- unconstrained measure (treat infinity as "give the child its desired size") ----

    TEST(flex_layout_manager, unconstrained_height_children_have_height)
    {
        flex_fixture fixture;
        const flex_child& child = fixture.add_child({100, 100});

        fixture.layout.measure(400, inf);

        EXPECT_EQ(fixture.layout.get_flex_frame(child).height, 100);
    }

    TEST(flex_layout_manager, unconstrained_width_children_have_width)
    {
        flex_fixture fixture;
        const flex_child& child = fixture.add_child({100, 100});

        fixture.layout.measure(inf, 400);

        EXPECT_EQ(fixture.layout.get_flex_frame(child).width, 100);
    }

    struct direction_case
    {
        double width_constraint;
        double height_constraint;
        flex_direction direction;
    };

    class unconstrained_measure_honors_direction : public ::testing::TestWithParam<direction_case>
    {
    };

    TEST_P(unconstrained_measure_honors_direction, frame_origin_is_zero)
    {
        const auto& p = GetParam();
        flex_fixture fixture;
        fixture.layout.set_direction(p.direction);
        const flex_child& child = fixture.add_child({100, 100});

        fixture.layout.measure(p.width_constraint, p.height_constraint);

        const rect frame = fixture.layout.get_flex_frame(child);
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.y, 0);
    }

    INSTANTIATE_TEST_SUITE_P(flex_layout_manager, unconstrained_measure_honors_direction,
                             ::testing::Values(direction_case{inf, 400, flex_direction::row_reverse},
                                               direction_case{inf, 400, flex_direction::row},
                                               direction_case{400, inf, flex_direction::column_reverse},
                                               direction_case{400, inf, flex_direction::column}));

    // ---- padding ----

    TEST(flex_layout_manager, padding_applied_row_direction)
    {
        constexpr double padding = 16;
        flex_fixture fixture;
        fixture.layout.set_padding(thickness(padding));
        const flex_child& view1 = fixture.add_child({150, 100});
        const flex_child& view2 = fixture.add_child({150, 100});

        const size measured = measure_and_arrange(fixture, 1000, 1000);

        const rect frame1 = view1.last_arrange;
        const rect frame2 = view2.last_arrange;
        EXPECT_EQ(frame1.x, padding);                        // left padding
        EXPECT_EQ(frame1.y, padding);                        // top padding
        EXPECT_EQ(measured.width - frame2.right(), padding); // right padding
    }

    TEST(flex_layout_manager, padding_applied_column_direction)
    {
        constexpr double padding = 16;
        flex_fixture fixture;
        fixture.layout.set_padding(thickness(padding));
        fixture.layout.set_direction(flex_direction::column);
        const flex_child& view1 = fixture.add_child({150, 100});
        const flex_child& view2 = fixture.add_child({150, 100});

        const size measured = measure_and_arrange(fixture, 1000, 1000);

        const rect frame1 = view1.last_arrange;
        const rect frame2 = view2.last_arrange;
        EXPECT_EQ(frame1.y, padding);                          // top padding
        EXPECT_EQ(measured.height - frame2.bottom(), padding); // bottom padding
    }

    // ---- margins (ported from FlexLayoutMarginTests; the engine is shared) ----

    TEST(flex_layout_manager, margin_left_row)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        flex_child& child = fixture.add_child({10, 0}, thickness(10, 0, 0, 0));
        child.set_width_request(10);

        measure_and_arrange(fixture, 100, 100);

        EXPECT_EQ(child.last_arrange, rect(10, 0, 10, 100));
    }

    TEST(flex_layout_manager, margin_top_column)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::column);
        flex_child& child = fixture.add_child({0, 10}, thickness(0, 10, 0, 0));
        child.set_height_request(10);

        measure_and_arrange(fixture, 100, 100);

        EXPECT_EQ(child.last_arrange, rect(0, 10, 100, 10));
    }

    TEST(flex_layout_manager, margin_right_row_justify_end)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        fixture.layout.set_justify_content(flex_justify::end);
        flex_child& child = fixture.add_child({10, 0}, thickness(0, 0, 10, 0));
        child.set_width_request(10);

        measure_and_arrange(fixture, 100, 100);

        EXPECT_EQ(child.last_arrange, rect(80, 0, 10, 100));
    }

    TEST(flex_layout_manager, margin_and_flex_row)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        flex_child& child = fixture.add_child({0, 0}, thickness(10, 0, 10, 0));
        fixture.layout.set_grow(child, 1);

        measure_and_arrange(fixture, 100, 100);

        EXPECT_EQ(child.last_arrange, rect(10, 0, 80, 100));
    }

    TEST(flex_layout_manager, margin_with_sibling_row)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        flex_child& view0 = fixture.add_child({0, 0}, thickness(0, 0, 10, 0));
        flex_child& view1 = fixture.add_child({0, 0});
        fixture.layout.set_grow(view0, 1);
        fixture.layout.set_grow(view1, 1);

        measure_and_arrange(fixture, 100, 100);

        EXPECT_EQ(view0.last_arrange, rect(0, 0, 45, 100));
        EXPECT_EQ(view1.last_arrange, rect(55, 0, 45, 100));
    }

    // ---- ordering (ported from FlexOrderTests) ----

    TEST(flex_layout_manager, ordering_elements_column)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::column);
        flex_child& c0 = fixture.add_child({912, 20});
        flex_child& c1 = fixture.add_child({912, 20});
        flex_child& c2 = fixture.add_child({912, 20});
        flex_child& c3 = fixture.add_child({912, 20});
        for (auto* child : {&c0, &c1, &c2, &c3})
        {
            child->set_height_request(20);
        }
        fixture.layout.set_order(c3, 0);
        fixture.layout.set_order(c2, 1);
        fixture.layout.set_order(c1, 2);
        fixture.layout.set_order(c0, 3);

        measure_and_arrange(fixture, 912, 912);

        EXPECT_EQ(c3.last_arrange.y, 0);
        EXPECT_EQ(c2.last_arrange.y, 20);
        EXPECT_EQ(c1.last_arrange.y, 40);
        EXPECT_EQ(c0.last_arrange.y, 60);
    }

    TEST(flex_layout_manager, stable_sort_preserves_insertion_order)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::column);
        flex_child& c0 = fixture.add_child({912, 20});
        flex_child& c1 = fixture.add_child({912, 20});
        flex_child& c2 = fixture.add_child({912, 20});
        flex_child& c3 = fixture.add_child({912, 20});
        for (auto* child : {&c0, &c1, &c2, &c3})
        {
            child->set_height_request(20);
        }
        // c1, c2, c3 share Order=0 (preserve insertion order); c0 has Order=1 (comes last).
        fixture.layout.set_order(c0, 1);

        measure_and_arrange(fixture, 912, 912);

        EXPECT_EQ(c1.last_arrange.y, 0);
        EXPECT_EQ(c2.last_arrange.y, 20);
        EXPECT_EQ(c3.last_arrange.y, 40);
        EXPECT_EQ(c0.last_arrange.y, 60);
    }

    // ---- grow / free-space distribution (issue #34464) ----

    TEST(flex_layout_manager, grow_items_distribute_free_space_equally)
    {
        flex_fixture fixture;
        flex_child& item1 = fixture.add_child({50, 50});  // narrower natural width
        flex_child& item2 = fixture.add_child({100, 50}); // wider natural width
        item1.set_width_request(-1);                      // auto (use measured)
        item2.set_width_request(-1);
        fixture.layout.set_grow(item1, 1);
        fixture.layout.set_shrink(item1, 0);
        fixture.layout.set_grow(item2, 1);
        fixture.layout.set_shrink(item2, 0);

        fixture.layout.measure(300, 200);

        // 300 container, 150 natural total, 150 free → +75 each: 50+75=125, 100+75=175.
        EXPECT_EQ(fixture.layout.get_flex_frame(item1).width, 125);
        EXPECT_EQ(fixture.layout.get_flex_frame(item2).width, 175);
    }

    // ---- arrange-only pass uses updated WidthRequest (issue #31109) ----

    TEST(flex_layout_manager, arrange_only_pass_uses_updated_width_request)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        flex_child& child = fixture.add_child({100, 50});
        child.set_width_request(200);

        const size measured = measure_and_arrange(fixture, 1000, 1000);
        (void)measured;
        EXPECT_EQ(fixture.layout.get_flex_frame(child).width, 200);

        // Change WidthRequest without re-measuring, then arrange only.
        child.set_width_request(300);
        fixture.layout.arrange(rect(0, 0, 1000, 1000));

        EXPECT_EQ(fixture.layout.get_flex_frame(child).width, 300);
    }

    // ---- visibility change re-flows siblings ----

    TEST(flex_layout_manager, visibility_change_reflows_siblings)
    {
        flex_fixture fixture;
        fixture.layout.set_direction(flex_direction::row);
        fixture.layout.set_align_items(flex_align_items::start);
        flex_child& view0 = fixture.add_child({150, 100});
        flex_child& view1 = fixture.add_child({150, 100});
        view0.set_width_request(150);
        view1.set_width_request(150);

        size measured = fixture.layout.measure(1000, 1000);
        fixture.layout.arrange(rect(0, 0, measured.width, measured.height));
        const double when_visible = view1.last_arrange.x;

        view0.visibility_value = maui::core::visibility::collapsed;

        measured = fixture.layout.measure(1000, 1000);
        fixture.layout.arrange(rect(0, 0, measured.width, measured.height));
        const double when_invisible = view1.last_arrange.x;

        EXPECT_NE(when_visible, when_invisible);
    }
} // namespace
