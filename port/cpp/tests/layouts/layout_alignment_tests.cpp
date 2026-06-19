// Tests for the per-axis layout alignment surface (View.HorizontalOptions / VerticalOptions ->
// IView.HorizontalLayoutAlignment / VerticalLayoutAlignment) and how the arrange-time ComputeFrame
// (VisualElement.ArrangeOverride -> LayoutExtensions.ComputeFrame) consumes it. Ported from
// src/Core/tests/UnitTests/Layouts/LayoutExtensionTests.cs (the ComputeFrame oracle) plus a grid-cell
// and stack cross-axis integration that proves the manager -> view<>::arrange chain honors Start/Center/
// End/Fill. The C# oracle for the surface is View.cs (HorizontalOptionsProperty/VerticalOptionsProperty,
// default LayoutOptions.Fill) and LayoutOptions.cs (the StackLayout-only Expands bit is NOT part of the
// IView contract, so the port stores the resolved layout_alignment directly).
#include "maui/controls/view.hpp"

#include <limits>

#include "maui/controls/grid.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::grid;
    using maui::core::layout_alignment;
    using maui::graphics::rect;
    using maui::graphics::size;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // A real view<> that runs the genuine view<>::arrange / compute_frame (unlike layouts::testing::
    // mock_view, which overrides arrange to record raw bounds). Its DesiredSize is configurable (the C#
    // LayoutExtensionTests substitute supplies element.DesiredSize); width/height/maximum come from the
    // real bindable size requests so the explicit-width-overrides-fill paths are exercised the real way.
    class aligned_view : public maui::controls::view<maui::core::i_view>
    {
    public:
        // Mirror the C# substitute: DesiredSize is configurable (the tests set element.DesiredSize). measure
        // returns it unchanged (and keeps it set) so a manager-driven measure pass — e.g. grid.measure —
        // does not clobber the configured size the way the handler-less view<>::measure would (it would
        // resolve a {0,0} content against unset requests). arrange is NOT overridden: the real view<>::
        // arrange / compute_frame runs, which is exactly what these tests exercise.
        void set_desired_size(size value)
        {
            configured_ = value;
            desired_size_ = value;
        }

        size measure(double /*width_constraint*/, double /*height_constraint*/) override
        {
            desired_size_ = configured_;
            return configured_;
        }

    private:
        size configured_{};
    };

    // ---- the surface: default Fill, settable, stored (View.HorizontalOptions / VerticalOptions) ----

    TEST(layout_alignment, defaults_to_fill)
    {
        const aligned_view view;
        const maui::core::i_view& as_view = view;
        EXPECT_EQ(as_view.horizontal_layout_alignment(), layout_alignment::fill);
        EXPECT_EQ(as_view.vertical_layout_alignment(), layout_alignment::fill);
    }

    TEST(layout_alignment, setters_round_trip_through_iview)
    {
        aligned_view view;
        view.set_horizontal_layout_alignment(layout_alignment::start);
        view.set_vertical_layout_alignment(layout_alignment::end);
        const maui::core::i_view& as_view = view;
        EXPECT_EQ(as_view.horizontal_layout_alignment(), layout_alignment::start);
        EXPECT_EQ(as_view.vertical_layout_alignment(), layout_alignment::end);
    }

    // ---- ComputeFrame: AlignHorizontal Start/Center/End within a 300-wide bound (C# AlignmentTestData,
    // the no-margin + X-offset rows; margin is always zero in the port so the margin rows reduce here) ----

    // arrange a view with the given desired size + horizontal alignment into bounds, return the frame.
    rect arrange_horizontal(layout_alignment alignment, size desired, rect bounds)
    {
        aligned_view view;
        view.set_desired_size(desired);
        view.set_horizontal_layout_alignment(alignment);
        view.arrange(bounds);
        return view.frame();
    }

    TEST(layout_alignment, horizontal_start_keeps_left_edge)
    {
        // desired 100 wide, bounds 300 wide from x=0: Start -> x=0, width=100 (sized to desired).
        const rect frame = arrange_horizontal(layout_alignment::start, {100, 50}, rect(0, 0, 300, 50));
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.width, 100);
    }

    TEST(layout_alignment, horizontal_center_centers)
    {
        const rect frame = arrange_horizontal(layout_alignment::center, {100, 50}, rect(0, 0, 300, 50));
        EXPECT_EQ(frame.x, 100); // (300 - 100) / 2
        EXPECT_EQ(frame.width, 100);
    }

    TEST(layout_alignment, horizontal_end_right_aligns)
    {
        const rect frame = arrange_horizontal(layout_alignment::end, {100, 50}, rect(0, 0, 300, 50));
        EXPECT_EQ(frame.x, 200); // 300 - 100
        EXPECT_EQ(frame.width, 100);
    }

    TEST(layout_alignment, horizontal_fill_stretches_to_bounds)
    {
        // Fill with no explicit width + infinite MaximumWidth -> consume the whole bound, x at the start.
        const rect frame = arrange_horizontal(layout_alignment::fill, {100, 50}, rect(0, 0, 300, 50));
        EXPECT_EQ(frame.x, 0);
        EXPECT_EQ(frame.width, 300);
    }

    TEST(layout_alignment, horizontal_alignment_respects_bounds_offset)
    {
        // C# AlignmentTestData X/Y-offset rows: a bounds origin of (10, …) shifts every alignment by 10.
        EXPECT_EQ(arrange_horizontal(layout_alignment::start, {100, 50}, rect(10, 0, 300, 50)).x, 10);
        EXPECT_EQ(arrange_horizontal(layout_alignment::center, {100, 50}, rect(10, 0, 300, 50)).x, 110);
        EXPECT_EQ(arrange_horizontal(layout_alignment::end, {100, 50}, rect(10, 0, 300, 50)).x, 210);
        EXPECT_EQ(arrange_horizontal(layout_alignment::fill, {100, 50}, rect(10, 0, 300, 50)).x, 10);
        EXPECT_EQ(arrange_horizontal(layout_alignment::fill, {100, 50}, rect(10, 0, 300, 50)).width, 300);
    }

    // ---- ComputeFrame: AlignVertical Start/Center/End within a 300-tall bound ----

    rect arrange_vertical(layout_alignment alignment, size desired, rect bounds)
    {
        aligned_view view;
        view.set_desired_size(desired);
        view.set_vertical_layout_alignment(alignment);
        view.arrange(bounds);
        return view.frame();
    }

    TEST(layout_alignment, vertical_start_keeps_top_edge)
    {
        const rect frame = arrange_vertical(layout_alignment::start, {50, 100}, rect(0, 0, 50, 300));
        EXPECT_EQ(frame.y, 0);
        EXPECT_EQ(frame.height, 100);
    }

    TEST(layout_alignment, vertical_center_centers)
    {
        const rect frame = arrange_vertical(layout_alignment::center, {50, 100}, rect(0, 0, 50, 300));
        EXPECT_EQ(frame.y, 100); // (300 - 100) / 2
        EXPECT_EQ(frame.height, 100);
    }

    TEST(layout_alignment, vertical_end_bottom_aligns)
    {
        const rect frame = arrange_vertical(layout_alignment::end, {50, 100}, rect(0, 0, 50, 300));
        EXPECT_EQ(frame.y, 200); // 300 - 100
        EXPECT_EQ(frame.height, 100);
    }

    TEST(layout_alignment, vertical_fill_stretches_to_bounds)
    {
        const rect frame = arrange_vertical(layout_alignment::fill, {50, 100}, rect(0, 0, 50, 300));
        EXPECT_EQ(frame.y, 0);
        EXPECT_EQ(frame.height, 300);
    }

    // ---- ComputeFrame: an explicit width/height (or finite max) overrides Fill, centering the view
    // (C# WidthOverridesFill / WidthOverridesFillFromCenter / HorizontalFillRespectsMaxWidth) ----

    TEST(layout_alignment, explicit_width_overrides_fill_and_centers)
    {
        // Fill + explicit Width=100 -> the view is sized to 100 and centered within the 300-wide bound.
        aligned_view view;
        view.set_desired_size({100, 50});
        view.set_horizontal_layout_alignment(layout_alignment::fill);
        view.set_width_request(100);
        view.set_height_request(50);
        view.arrange(rect(0, 0, 300, 300));
        EXPECT_EQ(view.frame().width, 100); // explicit width wins over Fill
        EXPECT_EQ(view.frame().x, 100);     // (300 / 2) - (100 / 2)
    }

    TEST(layout_alignment, fill_respects_finite_maximum_width)
    {
        // Fill + no explicit width but a finite MaximumWidth=100 -> width caps at min(max, bounds)=100,
        // centered (C# MaxWidthOverridesFromCenter).
        aligned_view view;
        view.set_desired_size({50, 50});
        view.set_horizontal_layout_alignment(layout_alignment::fill);
        view.set_maximum_width_request(100);
        view.arrange(rect(0, 0, 300, 300));
        EXPECT_EQ(view.frame().width, 100); // min(100, 300)
        EXPECT_EQ(view.frame().x, 100);     // (300 / 2) - (100 / 2)
    }

    TEST(layout_alignment, fill_consumes_min_of_bounds_and_max)
    {
        // C# HorizontalFillRespectsMaxWidth: when the bound is smaller than the max, the bound wins.
        aligned_view view;
        view.set_desired_size({50, 50});
        view.set_horizontal_layout_alignment(layout_alignment::fill);
        view.set_maximum_width_request(300);
        view.arrange(rect(0, 0, 200, 300)); // bounds 200 < max 300
        EXPECT_EQ(view.frame().width, 200); // min(300, 200)
    }

    // ---- the arrange return value is the resolved frame size, not the raw bounds (C# return Frame.Size) ----

    TEST(layout_alignment, arrange_returns_resolved_frame_size)
    {
        aligned_view view;
        view.set_desired_size({100, 80});
        view.set_horizontal_layout_alignment(layout_alignment::center);
        view.set_vertical_layout_alignment(layout_alignment::center);
        const size arranged = view.arrange(rect(0, 0, 300, 300));
        EXPECT_EQ(arranged.width, 100);
        EXPECT_EQ(arranged.height, 80);
    }

    // ---- integration: a real grid cell honors a child's HorizontalOptions/VerticalOptions ----
    // The grid manager passes the FULL cell bounds to child.arrange; the child's own view<>::arrange then
    // applies ComputeFrame. A 200x200 single-cell grid with a 100x100-desired child set to Center/Center
    // positions the child at (50,50) sized 100x100; the default (Fill) child fills the whole cell.

    // grid is non-copyable/non-movable (the element tree owns identity), so configure in place.
    void make_single_cell(grid& g)
    {
        g.add_row_definition(maui::core::grid_length{200});
        g.add_column_definition(maui::core::grid_length{200});
    }

    TEST(layout_alignment, grid_cell_centers_a_centered_child)
    {
        grid g;
        make_single_cell(g);
        aligned_view child;
        child.set_desired_size({100, 100});
        child.set_horizontal_layout_alignment(layout_alignment::center);
        child.set_vertical_layout_alignment(layout_alignment::center);
        g.add(child);
        g.set_row(child, 0);
        g.set_column(child, 0);

        g.measure(inf, inf);
        g.arrange(rect(0, 0, 200, 200));

        EXPECT_EQ(child.frame(), rect(50, 50, 100, 100)); // centered in the 200x200 cell
    }

    TEST(layout_alignment, grid_cell_end_aligns_an_end_child)
    {
        grid g;
        make_single_cell(g);
        aligned_view child;
        child.set_desired_size({100, 100});
        child.set_horizontal_layout_alignment(layout_alignment::end);
        child.set_vertical_layout_alignment(layout_alignment::end);
        g.add(child);
        g.set_row(child, 0);
        g.set_column(child, 0);

        g.measure(inf, inf);
        g.arrange(rect(0, 0, 200, 200));

        EXPECT_EQ(child.frame(), rect(100, 100, 100, 100)); // bottom-right of the 200x200 cell
    }

    TEST(layout_alignment, grid_cell_fills_a_default_child)
    {
        // The default (Fill) child stretches to the whole cell — the pre-existing behavior is preserved.
        grid g;
        make_single_cell(g);
        aligned_view child;
        child.set_desired_size({100, 100});
        g.add(child);
        g.set_row(child, 0);
        g.set_column(child, 0);

        g.measure(inf, inf);
        g.arrange(rect(0, 0, 200, 200));

        EXPECT_EQ(child.frame(), rect(0, 0, 200, 200)); // fills the cell
    }

    // Regression for the scroll_to_group layout: two 3-auto-row x 2-star-col grids + a readout in a vertical
    // stack must arrange sequentially without overlap — each grid reserves its FULL measured height in the
    // stack (a grid child's desired height is honored, mirroring C# StackLayout over a GridLayout).
    void make_three_auto_two_star(maui::controls::grid& g)
    {
        g.add_row_definition(maui::core::grid_length::automatic());
        g.add_row_definition(maui::core::grid_length::automatic());
        g.add_row_definition(maui::core::grid_length::automatic());
        g.add_column_definition(maui::core::grid_length::star());
        g.add_column_definition(maui::core::grid_length::star());
    }

    TEST(scroll_to_group_repro, two_auto_row_grids_stack_without_overlap)
    {
        maui::controls::vertical_stack_layout root;
        root.set_spacing(12);

        grid g1;
        make_three_auto_two_star(g1);
        aligned_view a0;
        aligned_view a1;
        aligned_view a2;
        for (auto* v : {&a0, &a1, &a2})
        {
            v->set_desired_size({80, 30});
        }
        g1.add(a0);
        g1.set_row(a0, 0);
        g1.set_column(a0, 0);
        g1.add(a1);
        g1.set_row(a1, 1);
        g1.set_column(a1, 0);
        g1.add(a2);
        g1.set_row(a2, 2);
        g1.set_column(a2, 0);

        grid g2;
        make_three_auto_two_star(g2);
        aligned_view b0;
        aligned_view b1;
        aligned_view b2;
        for (auto* v : {&b0, &b1, &b2})
        {
            v->set_desired_size({80, 30});
        }
        g2.add(b0);
        g2.set_row(b0, 0);
        g2.set_column(b0, 0);
        g2.add(b1);
        g2.set_row(b1, 1);
        g2.set_column(b1, 0);
        g2.add(b2);
        g2.set_row(b2, 2);
        g2.set_column(b2, 0);

        aligned_view readout;
        readout.set_desired_size({200, 20});

        root.add(g1);
        root.add(g2);
        root.add(readout);

        const size m = root.measure(400, inf);
        root.arrange(rect(0, 0, 400, m.height));

        // Each grid is 3 auto rows x 30 = 90 tall; stack spacing 12: 90 + 12 + 90 + 12 + 20 = 224.
        EXPECT_EQ(m.height, 224);
        EXPECT_EQ(g1.frame().y, 0);
        EXPECT_EQ(g1.frame().height, 90);
        EXPECT_EQ(g2.frame().y, 102); // 90 + 12 spacing — below g1
        EXPECT_EQ(g2.frame().height, 90);
        EXPECT_EQ(readout.frame().y, 204);                              // 192 + 12 — below g2
        EXPECT_GE(g2.frame().y, g1.frame().y + g1.frame().height);      // no overlap: g2 below g1
        EXPECT_GE(readout.frame().y, g2.frame().y + g2.frame().height); // no overlap: readout below g2
    }
} // namespace
