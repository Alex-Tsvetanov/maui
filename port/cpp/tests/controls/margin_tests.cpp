// Tests for the VisualElement/View.Margin seam on view<> (View.MarginProperty). Derived from the C#
// oracle: View.cs's MarginProperty (a Thickness, default zero, propertyChanged → InvalidateMeasureInternal
// (MarginChanged)) and the layout math in LayoutExtensions — ComputeDesiredSize ADDS the margin to the
// reported desired size, ComputeFrame SUBTRACTS it back out and offsets the frame by margin.Left/Top. The
// two halves must balance: measure adds, arrange subtracts. A real view<> (label) exercises the storage +
// the measure/arrange seam; a vertical_stack_layout exercises the parent re-layout after a margin change.
#include "maui/controls/border.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include <limits>

#include "maui/core/i_view.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::border;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::i_view;
    using maui::core::layout_alignment;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // ---- the storage seam (View.Margin round-trips; default zero) ----

    TEST(margin, defaults_to_zero)
    {
        // C# View.MarginProperty default is default(Thickness) — Thickness.Zero.
        label control;
        EXPECT_TRUE(static_cast<const i_view&>(control).margin().is_empty());
    }

    TEST(margin, round_trips)
    {
        label control;
        control.set_margin(thickness(5, 10, 15, 20));
        const thickness stored = static_cast<const i_view&>(control).margin();
        EXPECT_EQ(stored.left, 5);
        EXPECT_EQ(stored.top, 10);
        EXPECT_EQ(stored.right, 15);
        EXPECT_EQ(stored.bottom, 20);
    }

    // ---- (a) MEASURE includes the margin (C# LayoutExtensions.ComputeDesiredSize) ----

    TEST(margin, measure_includes_margin)
    {
        // ComputeDesiredSize: desired = measured content + margin. A 100x50 view (explicit Width/Height
        // requests) with a uniform 10 margin reports 120x70 — content + horizontal/vertical thickness (20).
        label control;
        control.set_width_request(100);
        control.set_height_request(50);
        control.set_margin(thickness(10));

        const size measured = control.measure(inf, inf);
        EXPECT_EQ(measured.width, 120);
        EXPECT_EQ(measured.height, 70);
        // desired_size() (read back through IView) reports the same margin-inclusive size.
        EXPECT_EQ(static_cast<const i_view&>(control).desired_size().width, 120);
        EXPECT_EQ(static_cast<const i_view&>(control).desired_size().height, 70);
    }

    TEST(margin, measure_zero_margin_is_unchanged)
    {
        // The default zero margin adds nothing — the desired size is exactly the resolved request, so the
        // prior (margin-less) behavior is preserved.
        label control;
        control.set_width_request(100);
        control.set_height_request(50);

        const size measured = control.measure(inf, inf);
        EXPECT_EQ(measured.width, 100);
        EXPECT_EQ(measured.height, 50);
    }

    // ---- (b) ARRANGE offsets by margin.Left/Top and shrinks the frame by the margin (ComputeFrame) ----

    TEST(margin, arrange_offsets_and_shrinks_frame)
    {
        // ComputeFrame: a Start-aligned view is positioned at bounds + margin.Left/Top, and its frame
        // shrinks by the margin (the margin-inclusive desired size minus the margin). An asymmetric margin
        // (left 10, top 20) shows each edge feeds its own axis.
        label control;
        control.set_width_request(100);
        control.set_height_request(50);
        control.set_horizontal_layout_alignment(layout_alignment::start);
        control.set_vertical_layout_alignment(layout_alignment::start);
        control.set_margin(thickness(10, 20, 0, 0)); // left, top, right, bottom
        control.measure(inf, inf);                   // desired = 110 x 70 (100 + 10, 50 + 20)

        control.arrange(rect(0, 0, 200, 200));
        const rect frame = control.frame();
        EXPECT_EQ(frame.x, 10);      // bounds.x + margin.left
        EXPECT_EQ(frame.y, 20);      // bounds.y + margin.top
        EXPECT_EQ(frame.width, 100); // desired 110 - margin.horizontal 10
        EXPECT_EQ(frame.height, 50); // desired 70  - margin.vertical 20
    }

    TEST(margin, arrange_fill_insets_frame_on_all_sides)
    {
        // A Fill view (the default alignment) with no explicit size consumes the bounds; the margin insets
        // the frame on every side — origin shifts by left/top, size shrinks by horizontal/vertical thickness.
        label control;
        control.set_margin(thickness(10));
        control.measure(inf, inf); // content 0, desired = 20 x 20 (just the margin)

        control.arrange(rect(0, 0, 200, 100));
        const rect frame = control.frame();
        EXPECT_EQ(frame.x, 10);      // bounds.x + margin.left
        EXPECT_EQ(frame.y, 10);      // bounds.y + margin.top
        EXPECT_EQ(frame.width, 180); // 200 - margin.horizontal 20
        EXPECT_EQ(frame.height, 80); // 100 - margin.vertical 20
    }

    // ---- (c) a margin change invalidates measure → the parent re-lays-out with the new reserved space ----

    TEST(margin, change_relays_out_parent)
    {
        // View.MarginPropertyChanged → InvalidateMeasureInternal(MarginChanged): the next parent layout pass
        // honors the new margin. A vertical stack arranges a Start-aligned child at its left edge; growing
        // the child's left margin shifts its arranged frame right by that margin on the re-layout.
        vertical_stack_layout stack;
        label child;
        child.set_width_request(100);
        child.set_height_request(50);
        child.set_horizontal_layout_alignment(layout_alignment::start);
        stack.add(child);

        stack.measure(inf, inf);
        stack.arrange(rect(0, 0, 200, 200));
        EXPECT_EQ(child.frame().x, 0); // zero margin, Start-aligned → flush left

        // Grow the left margin and re-run the parent layout pass. child.set_margin fires a REAL
        // invalidate_measure() now (view.hpp), but it reaches a window only via containing_window() — this
        // tree is a bare stack/label never mounted under a window, so it stays a no-op here and the test
        // still drives the re-layout by hand, exactly like a headless app whose host never installed a
        // relayout hook (see window::request_relayout / tests/hosting/relayout_tests.cpp for the mounted
        // case where the hook actually fires).
        child.set_margin(thickness(30, 0, 0, 0));
        stack.measure(inf, inf);
        stack.arrange(rect(0, 0, 200, 200));
        EXPECT_EQ(child.frame().x, 30);      // shifted right by the new left margin
        EXPECT_EQ(child.frame().width, 100); // still the explicit width (desired 130 - margin 30)
    }

    // ---- (d) the OVERRIDES must balance too — measure() overrides are where this breaks ----

    // border::measure is a full override of view<>::measure, so it does not inherit the margin fold and had
    // to add it explicitly. It did not, and the halves came apart: compute_frame subtracts the margin from
    // desired_size REGARDLESS (view.hpp:1069/1076), so a Border with a Margin lost 2x that margin. This is
    // the same defect layout.hpp:175-180 documents for the layout override, in a second override that never
    // got the fix. Zero margin is unaffected, which is why it survived — so both cases are pinned.
    TEST(margin, border_measure_includes_margin)
    {
        border bordered;
        bordered.set_padding(thickness(0));
        bordered.set_stroke_thickness(0);
        label content;
        content.set_width_request(100);
        content.set_height_request(50);
        bordered.set_content(content);

        bordered.set_margin(thickness(10));
        const size desired = bordered.measure(inf, inf);
        EXPECT_EQ(desired.width, 120);  // 100 content + margin.horizontal 20
        EXPECT_EQ(desired.height, 70);  // 50 content + margin.vertical 20

        // …and arrange gives the content box back, not content-minus-2x-margin. Alignment must be non-Fill
        // for the DESIRED size to govern the frame at all: compute_frame's Fill branch (view.hpp:1068-1073)
        // ignores desired_size and consumes min(bounds, maximum) instead, so a Fill Border reads 300-20=280
        // here whatever measure returned — it cannot witness this bug in either direction.
        bordered.set_horizontal_layout_alignment(layout_alignment::start);
        bordered.set_vertical_layout_alignment(layout_alignment::start);
        bordered.arrange(rect(0, 0, 300, 300));
        EXPECT_EQ(bordered.frame().width, 100);  // desired 120 - margin 20
        EXPECT_EQ(bordered.frame().height, 50);
        EXPECT_EQ(bordered.frame().x, 10);       // offset by margin.left
        EXPECT_EQ(bordered.frame().y, 10);
    }

    TEST(margin, border_zero_margin_is_unchanged)
    {
        border bordered;
        bordered.set_padding(thickness(0));
        bordered.set_stroke_thickness(0);
        label content;
        content.set_width_request(100);
        content.set_height_request(50);
        bordered.set_content(content);

        const size desired = bordered.measure(inf, inf);
        EXPECT_EQ(desired.width, 100);
        EXPECT_EQ(desired.height, 50);
    }

    // The margin must NOT be swallowed by an explicit size request: in C# the Width/HeightRequest clamp
    // happens inside the handler's GetDesiredSize and ComputeDesiredSize adds the margin to whatever that
    // returned, so the two are additive.
    TEST(margin, border_size_request_and_margin_are_additive)
    {
        border bordered;
        bordered.set_padding(thickness(0));
        bordered.set_stroke_thickness(0);
        bordered.set_width_request(100);
        bordered.set_height_request(50);
        bordered.set_margin(thickness(12));

        const size desired = bordered.measure(inf, inf);
        EXPECT_EQ(desired.width, 124);  // the requested 100 PLUS margin.horizontal 24
        EXPECT_EQ(desired.height, 74);
        bordered.arrange(rect(0, 0, 300, 300));
        EXPECT_EQ(bordered.frame().width, 100);  // and the frame is the requested size again
        EXPECT_EQ(bordered.frame().height, 50);
    }
} // namespace
