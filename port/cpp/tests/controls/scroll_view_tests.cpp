// Tests for the scroll_view control + its headless handler seam — ported from ScrollViewUnitTests.cs
// (src/Controls/tests/Core.UnitTests): TestConstructor / TestChildDoubleSet (content + parenting),
// TestOrientation (+DoubleSet, via the bindable property_changed signal), TestScrollTo /
// TestScrollToNotAnimated / TestScrollWasNotFiredOnNeither / TestBackToBackBiDirectionalScroll (the
// ScrollToRequested pipeline), and SetScrollPosition — plus the measure/arrange oracles (the
// handler-side CrossPlatformMeasure's unconstrained scrolling dimension and
// ArrangeContentUnbounded's overflow arrangement) and the headless seam: the platform mirrors
// orientation / bar visibilities / content / offsets, RECORDS every scroll_to request, clamps the
// target to the available range, writes the offsets back (firing Scrolled), and acknowledges
// completion (scroll_to_completed — the ScrollToAsync Task stand-in). The pending-request flush on
// handler attach ports ScrollView.OnHandlerChangedCore.
//
// NOT ported: the ScrollToAsync(Element, ScrollToPosition, animated) overload + TestScrollToInvalid
// (the element-coordinate walk is a documented deferral — scroll_view.hpp).
#include "maui/controls/scroll_view.hpp"

#include <cmath>
#include <memory>
#include <string_view>

#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::scroll_view;
    using maui::core::i_element_handler;
    using maui::core::scroll_bar_visibility;
    using maui::core::scroll_orientation;
    using maui::core::scroll_to_request;
    using maui::core::scroll_view_handler;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;

    // ---- the control in isolation ----

    TEST(scroll_view, defaults_match_the_csharp_property_defaults) // C# TestConstructor
    {
        const scroll_view scroller;
        EXPECT_EQ(scroller.content(), nullptr);
        EXPECT_EQ(scroller.orientation(), scroll_orientation::vertical);
        EXPECT_EQ(scroller.horizontal_scroll_bar_visibility(), scroll_bar_visibility::default_);
        EXPECT_EQ(scroller.vertical_scroll_bar_visibility(), scroll_bar_visibility::default_);
        EXPECT_EQ(scroller.scroll_x(), 0.0);
        EXPECT_EQ(scroller.scroll_y(), 0.0);
        EXPECT_EQ(scroller.content_size(), size(0, 0));
    }

    TEST(scroll_view, content_set_replace_and_clear_reparents) // C# TestConstructor + TestChildDoubleSet
    {
        scroll_view scroller;
        mock_view child;

        scroller.set_content(child);
        EXPECT_EQ(scroller.content(), &child);
        EXPECT_EQ(child.logical_parent(), &scroller);

        scroller.set_content(child); // double-set is a no-op
        EXPECT_EQ(scroller.content(), &child);

        scroller.set_content(nullptr);
        EXPECT_EQ(scroller.content(), nullptr);
        EXPECT_EQ(child.logical_parent(), nullptr);
    }

    TEST(scroll_view, orientation_changes_signal_the_property) // C# TestOrientation
    {
        scroll_view scroller;
        EXPECT_EQ(scroller.orientation(), scroll_orientation::vertical);

        bool signaled = false;
        scroller.property_changed.connect([&signaled](std::string_view name) {
            if (name == "orientation")
            {
                signaled = true;
            }
        });

        scroller.set_orientation(scroll_orientation::horizontal);
        EXPECT_EQ(scroller.orientation(), scroll_orientation::horizontal);
        EXPECT_TRUE(signaled);

        scroller.set_orientation(scroll_orientation::both);
        EXPECT_EQ(scroller.orientation(), scroll_orientation::both);

        scroller.set_orientation(scroll_orientation::neither);
        EXPECT_EQ(scroller.orientation(), scroll_orientation::neither);
    }

    TEST(scroll_view, orientation_double_set_does_not_signal) // C# TestOrientationDoubleSet
    {
        scroll_view scroller;
        bool signaled = false;
        scroller.property_changed.connect([&signaled](std::string_view name) {
            if (name == "orientation")
            {
                signaled = true;
            }
        });

        scroller.set_orientation(scroller.orientation());
        EXPECT_FALSE(signaled);
    }

    TEST(scroll_view, scroll_to_async_requests_an_animated_scroll) // C# TestScrollTo
    {
        scroll_view scroller;
        mock_view child;
        scroller.set_content(child);

        bool requested = false;
        scroller.scroll_to_requested.connect([&requested](const scroll_to_request& args) {
            requested = true;
            EXPECT_EQ(args.vertical_offset, 100.0);
            EXPECT_EQ(args.horizontal_offset, 0.0);
            EXPECT_FALSE(args.instant); // ShouldAnimate == true
        });

        EXPECT_TRUE(scroller.scroll_to_async(0, 100, true));
        EXPECT_TRUE(requested);
    }

    TEST(scroll_view, scroll_to_async_not_animated_is_instant) // C# TestScrollToNotAnimated
    {
        scroll_view scroller;
        mock_view child;
        scroller.set_content(child);

        bool requested = false;
        scroller.scroll_to_requested.connect([&requested](const scroll_to_request& args) {
            requested = true;
            EXPECT_EQ(args.vertical_offset, 100.0);
            EXPECT_EQ(args.horizontal_offset, 0.0);
            EXPECT_TRUE(args.instant); // ShouldAnimate == false
        });

        EXPECT_TRUE(scroller.scroll_to_async(0, 100, false));
        EXPECT_TRUE(requested);
    }

    TEST(scroll_view, scroll_to_async_is_suppressed_on_neither) // C# TestScrollWasNotFiredOnNeither
    {
        scroll_view scroller;
        scroller.set_orientation(scroll_orientation::neither);
        mock_view child;
        scroller.set_content(child);

        bool requested = false;
        scroller.scroll_to_requested.connect([&requested](const scroll_to_request&) { requested = true; });

        EXPECT_FALSE(scroller.scroll_to_async(0, 100, true));
        EXPECT_FALSE(requested);
    }

    TEST(scroll_view, back_to_back_bidirectional_scrolls_each_fire_once) // C# TestBackToBackBiDirectionalScroll
    {
        scroll_view scroller;
        scroller.set_orientation(scroll_orientation::both);
        mock_view child;
        child.configure({1000, 1000});
        scroller.set_content(child);

        int y100_count = 0;
        scroller.scroll_to_requested.connect([&y100_count](const scroll_to_request& args) {
            if (args.vertical_offset == 100.0)
            {
                ++y100_count;
            }
        });

        scroller.scroll_to_async(100, 100, true);
        EXPECT_EQ(y100_count, 1);

        scroller.scroll_to_async(0, 100, true);
        EXPECT_EQ(y100_count, 2);
    }

    TEST(scroll_view, set_scrolled_position_updates_and_fires_scrolled) // C# SetScrollPosition
    {
        scroll_view scroller;
        int scrolled_count = 0;
        double last_x = -1;
        double last_y = -1;
        scroller.scrolled.connect([&](double x, double y) {
            ++scrolled_count;
            last_x = x;
            last_y = y;
        });

        scroller.set_scrolled_position(100, 100);
        EXPECT_EQ(scroller.scroll_x(), 100.0);
        EXPECT_EQ(scroller.scroll_y(), 100.0);
        EXPECT_EQ(scrolled_count, 1);
        EXPECT_EQ(last_x, 100.0);
        EXPECT_EQ(last_y, 100.0);

        scroller.set_scrolled_position(100, 100); // unchanged -> no event
        EXPECT_EQ(scrolled_count, 1);
    }

    // ---- measure/arrange (the CrossPlatformMeasure / ArrangeContentUnbounded oracles) ----

    TEST(scroll_view, measure_unconstrains_the_scrolling_dimension)
    {
        scroll_view scroller; // vertical (default)
        mock_view child;
        child.configure({50, 500});
        scroller.set_content(child);

        const size measured = scroller.measure(100, 100);
        // The content saw an unconstrained height (vertical scroll) and the incoming width.
        EXPECT_EQ(child.last_measure_width, 100.0);
        EXPECT_TRUE(std::isinf(child.last_measure_height));
        // The result clamps to the constraints (content 50x500 -> 50x100).
        EXPECT_EQ(measured.width, 50.0);
        EXPECT_EQ(measured.height, 100.0);
    }

    TEST(scroll_view, measure_with_padding_insets_the_content)
    {
        scroll_view scroller;
        scroller.set_padding(thickness(10));
        mock_view child;
        child.configure({50, 30});
        scroller.set_content(child);

        const size measured = scroller.measure(1000, 1000);
        EXPECT_EQ(child.last_measure_width, 980.0); // width constrained minus the padding
        EXPECT_EQ(measured.width, 70.0);            // content + padding
        EXPECT_EQ(measured.height, 50.0);
    }

    TEST(scroll_view, arrange_lets_the_content_overflow_and_tracks_content_size)
    {
        scroll_view scroller; // vertical
        mock_view child;
        child.configure({50, 500});
        scroller.set_content(child);

        scroller.measure(100, 100);
        scroller.arrange(rect(0, 0, 100, 100));

        // ArrangeContentUnbounded: the content arranges into max(bounds, desired) per dimension.
        EXPECT_EQ(child.last_arrange, rect(0, 0, 100, 500));
        // ContentSize follows the arranged content frame (+ margin, zero here).
        EXPECT_EQ(scroller.content_size(), size(100, 500));
    }

    TEST(scroll_view, arrange_places_content_host_relative_at_a_non_zero_origin)
    {
        // The scroller's content is a native subview of the UIScrollView, so its frame is expressed in the
        // scroller's own content coordinate space (origin 0), NOT the page origin. Arranging the scroller at
        // a non-zero page origin must still arrange the content at the scroller-relative origin (the padding
        // inset), never at bounds.x/bounds.y — otherwise iOS double-offsets it (the border_playground gap).
        scroll_view scroller; // vertical
        scroller.set_padding(thickness(5));
        mock_view child;
        child.configure({50, 500});
        scroller.set_content(child);

        scroller.measure(100, 200);
        scroller.arrange(rect(16, 216, 100, 200)); // framed at a non-zero page origin

        // Host-relative: origin at the padding inset (5,5), NOT (16+5, 216+5). Height overflows (content 500
        // + 2*5 padding = 510 > the 200 viewport) so the inner arrange height is 510 - 10 = 500; the width
        // fills the viewport (100 - 10 padding = 90).
        EXPECT_EQ(child.last_arrange, rect(5, 5, 90, 500));
        // ContentSize follows the arranged content frame (+ margin, zero here) — origin-independent.
        EXPECT_EQ(scroller.content_size(), size(90, 500));
    }

    TEST(scroll_view, measure_without_content_resets_content_size)
    {
        scroll_view scroller;
        mock_view child;
        child.configure({50, 500});
        scroller.set_content(child);
        scroller.measure(100, 100);
        scroller.arrange(rect(0, 0, 100, 100));
        EXPECT_EQ(scroller.content_size(), size(100, 500));

        scroller.set_content(nullptr);
        scroller.measure(100, 100);
        EXPECT_EQ(scroller.content_size(), size(0, 0));
    }

    // ---- the handler seam (headless mirrors + the recorded scroll_to trail) ----

    TEST(scroll_view_seam, attaching_handler_mirrors_the_scroll_surface)
    {
        scroll_view scroller;
        scroller.set_orientation(scroll_orientation::horizontal);
        scroller.set_horizontal_scroll_bar_visibility(scroll_bar_visibility::always);
        scroller.set_vertical_scroll_bar_visibility(scroll_bar_visibility::never);
        mock_view child;
        scroller.set_content(child);

        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        EXPECT_EQ(platform->hosted_content, &child);
        EXPECT_EQ(platform->orientation, scroll_orientation::horizontal);
        EXPECT_EQ(platform->horizontal_bar_visibility, scroll_bar_visibility::always);
        EXPECT_EQ(platform->vertical_bar_visibility, scroll_bar_visibility::never);

        scroller.set_orientation(scroll_orientation::both); // runtime change flows through the mapper
        EXPECT_EQ(platform->orientation, scroll_orientation::both);
    }

    TEST(scroll_view_seam, scroll_to_records_clamps_writes_back_and_completes)
    {
        scroll_view scroller;
        mock_view child;
        child.configure({100, 1000});
        scroller.set_content(child);

        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        scroller.measure(100, 100);
        scroller.arrange(rect(0, 0, 100, 100)); // viewport 100x100, content 100x1000

        int completed = 0;
        scroller.scroll_to_completed.connect([&completed] { ++completed; });
        int scrolled_count = 0;
        scroller.scrolled.connect([&scrolled_count](double /*x*/, double /*y*/) { ++scrolled_count; });

        scroller.scroll_to_async(0, 500, false);
        ASSERT_EQ(platform->scroll_requests.size(), 1U);
        EXPECT_EQ(platform->scroll_requests[0].vertical_offset, 500.0);
        EXPECT_TRUE(platform->scroll_requests[0].instant);
        EXPECT_EQ(platform->offset_y, 500.0);
        EXPECT_EQ(scroller.scroll_y(), 500.0); // the platform write-back updated ScrollY
        EXPECT_EQ(scrolled_count, 1);
        EXPECT_EQ(completed, 1);

        // A target beyond the scrollable range clamps to ContentSize - Frame (1000 - 100).
        scroller.scroll_to_async(0, 5000, false);
        ASSERT_EQ(platform->scroll_requests.size(), 2U);
        EXPECT_EQ(platform->offset_y, 900.0);
        EXPECT_EQ(scroller.scroll_y(), 900.0);
        EXPECT_EQ(completed, 2);
    }

    TEST(scroll_view_seam, pending_scroll_to_flushes_on_handler_attach) // C# OnHandlerChangedCore
    {
        scroll_view scroller;
        mock_view child;
        child.configure({100, 1000});
        scroller.set_content(child);

        scroller.scroll_to_async(0, 100, true); // no handler yet -> pended

        auto handler = std::make_shared<scroll_view_handler>();
        scroller.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        ASSERT_EQ(platform->scroll_requests.size(), 1U);
        EXPECT_EQ(platform->scroll_requests[0].vertical_offset, 100.0);
        EXPECT_FALSE(platform->scroll_requests[0].instant);
    }

    TEST(scroll_view_seam, handler_resolved_from_default_registry)
    {
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<scroll_view>();
        ASSERT_NE(handler, nullptr);
        EXPECT_NE(dynamic_cast<scroll_view_handler*>(handler.get()), nullptr);
    }

    // ---- U-SA: SafeAreaEdges ----

    TEST(scroll_view_safe_area, safe_area_edges_defaults_to_default)
    {
        const maui::controls::scroll_view sv;
        EXPECT_EQ(sv.safe_area_edges(), maui::core::safe_area_edges::default_edges());
    }

    TEST(scroll_view_safe_area, safe_area_edges_is_settable)
    {
        maui::controls::scroll_view sv;
        sv.set_safe_area_edges(maui::core::safe_area_edges::none());
        EXPECT_EQ(sv.safe_area_edges(), maui::core::safe_area_edges::none());
    }

    TEST(scroll_view_safe_area, get_regions_for_edge_returns_property_as_is)
    {
        maui::controls::scroll_view sv;
        sv.set_safe_area_edges(maui::core::safe_area_edges::none());
        const maui::core::i_safe_area_view2& sv2 = sv;
        EXPECT_EQ(sv2.get_safe_area_regions_for_edge(0), maui::core::safe_area_regions::none);
        sv.set_safe_area_edges(maui::core::safe_area_edges::default_edges());
        EXPECT_EQ(sv2.get_safe_area_regions_for_edge(1), maui::core::safe_area_regions::default_value);
    }

    TEST(scroll_view_safe_area, set_safe_area_insets_does_not_disturb_the_edges_property)
    {
        maui::controls::scroll_view sv;
        maui::core::i_safe_area_view2& sv2 = sv;
        const maui::core::thickness t{1.0, 2.0, 3.0, 4.0};
        sv2.set_safe_area_insets(t);
        EXPECT_EQ(sv.safe_area_edges(), maui::core::safe_area_edges::default_edges());
    }

    // ---- MauiScrollView's safe-area application (MauiScrollView.cs:383-386, 432-455) ----
    //
    // The branch that matters: iOS only sets AdjustedContentInset when the content OVERFLOWS the scroll
    // view. When the content FITS, iOS leaves it zero — and MauiScrollView then computes the inset ITSELF
    // (GetInset) and arranges the content at the inset ORIGIN. Without this a page whose content fits its
    // window renders flush to the top, under the bars/notch: measured as a real 32px (41pt x the 0.77
    // Catalyst scale) offset vs MAUI on slider / picker / layout_is_enabled.

    TEST(scroll_view_safe_area_apply, arranges_content_at_the_inset_origin_when_the_system_did_not_adjust)
    {
        maui::controls::scroll_view sv;
        maui::layouts::testing::mock_view content;
        content.configure({100, 100});
        sv.set_content(content);
        // The system did NOT adjust (content fits) => MauiScrollView computes the inset itself.
        sv.set_system_adjusted_content_inset(maui::core::thickness{});
        static_cast<maui::core::i_safe_area_view2&>(sv).set_safe_area_insets(maui::core::thickness{0, 41, 0, 0});

        sv.measure(200, 400);
        sv.arrange(maui::graphics::rect(0, 0, 200, 400));

        EXPECT_DOUBLE_EQ(content.last_arrange.y, 41.0); // the inset ORIGIN is honored
    }

    TEST(scroll_view_safe_area_apply, keeps_content_at_the_origin_when_the_system_already_adjusted)
    {
        maui::controls::scroll_view sv;
        maui::layouts::testing::mock_view content;
        content.configure({100, 100});
        sv.set_content(content);
        // The system DID adjust (content overflows) => UIKit's contentInset supplies the visual offset, so
        // the content arranges at 0-origin with only the SIZE reduced. Re-adding the origin here would
        // double-count against UIKit's own inset.
        sv.set_system_adjusted_content_inset(maui::core::thickness{0, 41, 0, 0});
        static_cast<maui::core::i_safe_area_view2&>(sv).set_safe_area_insets(maui::core::thickness{0, 41, 0, 0});

        sv.measure(200, 400);
        sv.arrange(maui::graphics::rect(0, 0, 200, 400));

        EXPECT_DOUBLE_EQ(content.last_arrange.y, 0.0);
    }

    TEST(scroll_view_safe_area_apply, no_inset_when_the_edges_are_none)
    {
        maui::controls::scroll_view sv;
        maui::layouts::testing::mock_view content;
        content.configure({100, 100});
        sv.set_content(content);
        sv.set_safe_area_edges(maui::core::safe_area_edges::none()); // GetManualInsetForEdge => 0
        sv.set_system_adjusted_content_inset(maui::core::thickness{});
        static_cast<maui::core::i_safe_area_view2&>(sv).set_safe_area_insets(maui::core::thickness{0, 41, 0, 0});

        sv.measure(200, 400);
        sv.arrange(maui::graphics::rect(0, 0, 200, 400));

        EXPECT_DOUBLE_EQ(content.last_arrange.y, 0.0);
    }

    TEST(scroll_view_safe_area_apply, no_inset_when_nothing_was_pushed)
    {
        maui::controls::scroll_view sv;
        maui::layouts::testing::mock_view content;
        content.configure({100, 100});
        sv.set_content(content);

        sv.measure(200, 400);
        sv.arrange(maui::graphics::rect(0, 0, 200, 400));

        EXPECT_DOUBLE_EQ(content.last_arrange.y, 0.0); // headless pushes nothing => zero => no adjustment
    }

    // MauiScrollView.CrossPlatformMeasure (MauiScrollView.cs:548-562) — the same subtract-then-add-back
    // as MauiView, so the scroll view still reports the full span it occupies.
    TEST(scroll_view_safe_area_apply, measure_subtracts_the_safe_area_then_adds_it_back)
    {
        maui::controls::scroll_view sv;
        maui::layouts::testing::mock_view content;
        content.configure({100, 100});
        sv.set_content(content);
        sv.set_system_adjusted_content_inset(maui::core::thickness{});
        static_cast<maui::core::i_safe_area_view2&>(sv).set_safe_area_insets(maui::core::thickness{10, 41, 30, 0});

        const maui::graphics::size measured = sv.measure(200, 400);

        EXPECT_DOUBLE_EQ(content.last_measure_width, 160.0); // 200 - 10 - 30
        EXPECT_DOUBLE_EQ(measured.width, 140.0);             // the content's 100 + 10 + 30 back
        EXPECT_DOUBLE_EQ(measured.height, 141.0);            // the content's 100 + 41 back
    }
} // namespace
