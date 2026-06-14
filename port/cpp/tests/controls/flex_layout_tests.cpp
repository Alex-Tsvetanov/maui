// Tests for the flex_layout control + its headless handler seam. Verifies (1) the container-level flex
// knobs default + round-trip (C# FlexLayout.*Property defaults: Direction=Row, JustifyContent=Start,
// AlignContent/AlignItems=Stretch, Position=Relative, Wrap=NoWrap); (2) the per-child attached values
// default + round-trip + validate (Grow/Shrink >= 0 rejected like C#'s validateValue); (3) geometry
// through the control reproduces the flex layout; and (4) the handler is self-registered + the panel
// child count tracks mutations. Ported from src/Controls/tests/Core.UnitTests/Layouts/FlexLayoutTests.cs
// (control surface + the unconstrained / grow / padding cases) driven through the control.
#include "maui/controls/flex_layout.hpp"

#include <memory>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_flex_layout.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/layouts/flex_basis.hpp"
#include "maui/layouts/flex_enums.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::flex_layout;
    using maui::core::default_handler_registry;
    using maui::core::i_element_handler;
    using maui::core::i_flex_layout;
    using maui::core::i_layout;
    using maui::core::layout_handler;
    using maui::graphics::rect;
    using maui::layouts::flex_align_content;
    using maui::layouts::flex_align_items;
    using maui::layouts::flex_align_self;
    using maui::layouts::flex_basis;
    using maui::layouts::flex_direction;
    using maui::layouts::flex_justify;
    using maui::layouts::flex_position;
    using maui::layouts::flex_wrap;
    using maui::layouts::testing::mock_view;

    // ---- container-level knobs: defaults + round-trip ----

    TEST(flex_layout_control, container_knobs_default_to_csharp_defaults)
    {
        flex_layout layout;
        EXPECT_EQ(layout.direction(), flex_direction::row);
        EXPECT_EQ(layout.justify_content(), flex_justify::start);
        EXPECT_EQ(layout.align_content(), flex_align_content::stretch);
        EXPECT_EQ(layout.align_items(), flex_align_items::stretch);
        EXPECT_EQ(layout.position(), flex_position::relative);
        EXPECT_EQ(layout.wrap(), flex_wrap::no_wrap);
    }

    TEST(flex_layout_control, container_knobs_round_trip)
    {
        flex_layout layout;
        layout.set_direction(flex_direction::column);
        layout.set_justify_content(flex_justify::center);
        layout.set_align_content(flex_align_content::end);
        layout.set_align_items(flex_align_items::center);
        layout.set_position(flex_position::absolute);
        layout.set_wrap(flex_wrap::wrap);

        EXPECT_EQ(layout.direction(), flex_direction::column);
        EXPECT_EQ(layout.justify_content(), flex_justify::center);
        EXPECT_EQ(layout.align_content(), flex_align_content::end);
        EXPECT_EQ(layout.align_items(), flex_align_items::center);
        EXPECT_EQ(layout.position(), flex_position::absolute);
        EXPECT_EQ(layout.wrap(), flex_wrap::wrap);
    }

    // ---- per-child attached values: defaults + round-trip + validation ----

    TEST(flex_layout_control, unset_child_uses_default_attached_values)
    {
        flex_layout layout;
        mock_view child;
        layout.add(child);

        EXPECT_EQ(layout.get_order(child), 0);
        EXPECT_EQ(layout.get_grow(child), 0.0F);
        EXPECT_EQ(layout.get_shrink(child), 1.0F);
        EXPECT_EQ(layout.get_align_self(child), flex_align_self::auto_);
        EXPECT_TRUE(layout.get_basis(child).is_auto());
    }

    TEST(flex_layout_control, attached_values_round_trip)
    {
        flex_layout layout;
        mock_view child;
        layout.add(child);
        layout.set_order(child, 3);
        layout.set_grow(child, 2.0F);
        layout.set_shrink(child, 0.5F);
        layout.set_align_self(child, flex_align_self::center);
        layout.set_basis(child, flex_basis{0.4F, true});

        EXPECT_EQ(layout.get_order(child), 3);
        EXPECT_EQ(layout.get_grow(child), 2.0F);
        EXPECT_EQ(layout.get_shrink(child), 0.5F);
        EXPECT_EQ(layout.get_align_self(child), flex_align_self::center);
        EXPECT_TRUE(layout.get_basis(child).is_relative());
        EXPECT_EQ(layout.get_basis(child).length(), 0.4F);
    }

    TEST(flex_layout_control, negative_grow_or_shrink_is_ignored)
    {
        flex_layout layout;
        mock_view child;
        layout.add(child);
        layout.set_grow(child, 5.0F);
        layout.set_shrink(child, 3.0F);

        layout.set_grow(child, -1.0F);   // invalid (< 0): C# validateValue rejects
        layout.set_shrink(child, -2.0F); // invalid (< 0)

        EXPECT_EQ(layout.get_grow(child), 5.0F);
        EXPECT_EQ(layout.get_shrink(child), 3.0F);
    }

    // ---- geometry through the control ----

    TEST(flex_layout_control, two_children_row_stretch_fill_height)
    {
        flex_layout layout; // default Direction=Row, AlignItems=Stretch
        mock_view a;
        mock_view b;
        a.configure({100, 20});
        b.configure({100, 20});
        layout.add(a);
        layout.add(b);

        layout.measure(912, 912);
        layout.arrange(rect(0, 0, 912, 912));

        // Row: natural widths 100 each, stretched to full height.
        EXPECT_EQ(a.last_arrange, rect(0, 0, 100, 912));
        EXPECT_EQ(b.last_arrange, rect(100, 0, 100, 912));
    }

    TEST(flex_layout_control, grow_fills_available_space)
    {
        flex_layout layout;
        layout.set_direction(flex_direction::row);
        mock_view child;
        child.configure({0, 0});
        layout.add(child);
        layout.set_grow(child, 1.0F);

        layout.measure(100, 100);
        layout.arrange(rect(0, 0, 100, 100));

        EXPECT_EQ(child.last_arrange, rect(0, 0, 100, 100));
    }

    // ---- handler seam ----

    TEST(flex_layout_seam, panel_child_count_tracks_mutations)
    {
        flex_layout layout;
        auto handler = std::make_shared<layout_handler>();
        layout.set_handler(handler);
        auto* platform = handler->typed_platform_view();

        mock_view a;
        mock_view b;
        layout.add(a);
        layout.add(b);
        EXPECT_EQ(platform->children.size(), 2U);

        layout.remove_at(0);
        EXPECT_EQ(platform->children.size(), 1U);
        EXPECT_EQ(platform->children[0], &b);

        layout.clear();
        EXPECT_EQ(platform->children.size(), 0U);
    }

    TEST(flex_layout_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler = default_handler_registry().create_handler<flex_layout>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<layout_handler*>(handler.get()), nullptr);
    }

    TEST(flex_layout_control, usable_through_layout_interface)
    {
        flex_layout layout;
        mock_view child;
        layout.add(child);

        i_layout& as_layout = layout;
        EXPECT_EQ(as_layout.count(), 1);

        i_flex_layout& as_flex = layout;
        EXPECT_EQ(as_flex.direction(), flex_direction::row);
    }
} // namespace
