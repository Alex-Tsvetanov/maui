// Tests for the VisualElement size-request surface on view<> (WidthRequest / HeightRequest / Minimum* /
// Maximum*) and the per-child measure clamp. Derived from VisualElement.cs's IView.Width/Height/Minimum*/
// Maximum* mapping and the ViewHandlerExtensions.GetDesiredSizeFromHandler clamp. A label is used because
// its headless handler returns a content size (~7pt/char, 16pt tall), so the per-child clamp is observable.
#include "maui/controls/label.hpp"

#include <cmath>
#include <limits>
#include <memory>

#include "maui/core/i_view.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::label;
    using maui::core::i_view;
    using maui::core::label_handler;
    using maui::graphics::size;

    constexpr double inf = std::numeric_limits<double>::infinity();

    // ---- view<> as IView: the size getters derive from the requests, not the arranged frame ----

    TEST(size_request, defaults_match_visual_element)
    {
        label control;
        const i_view& view = control;
        // Width/Height: unset request -> Unset (NaN); Minimum*: unset -> Unset; Maximum*: +inf.
        EXPECT_TRUE(std::isnan(view.width()));
        EXPECT_TRUE(std::isnan(view.height()));
        EXPECT_TRUE(std::isnan(view.minimum_width()));
        EXPECT_TRUE(std::isnan(view.minimum_height()));
        EXPECT_EQ(view.maximum_width(), inf);
        EXPECT_EQ(view.maximum_height(), inf);
        // The request accessors report the raw defaults (-1 / +inf).
        EXPECT_EQ(control.width_request(), -1);
        EXPECT_EQ(control.height_request(), -1);
        EXPECT_EQ(control.minimum_width_request(), -1);
        EXPECT_EQ(control.maximum_width_request(), inf);
    }

    TEST(size_request, explicit_minus_one_width_is_unset)
    {
        // VisualElement.IView.Width: an explicitly-set -1 still reads as Unset ("size to content").
        label control;
        control.set_width_request(-1);
        control.set_height_request(-1);
        EXPECT_TRUE(std::isnan(static_cast<const i_view&>(control).width()));
        EXPECT_TRUE(std::isnan(static_cast<const i_view&>(control).height()));
    }

    TEST(size_request, explicit_request_reported_via_iview)
    {
        label control;
        control.set_width_request(120);
        control.set_height_request(40);
        EXPECT_EQ(static_cast<const i_view&>(control).width(), 120);
        EXPECT_EQ(static_cast<const i_view&>(control).height(), 40);
    }

    TEST(size_request, negative_request_clamps_to_zero)
    {
        // VisualElement.EnsurePositive: a negative (non -1) request reads as 0; Minimum* and Maximum* too.
        label control;
        control.set_width_request(-5);
        control.set_minimum_width_request(-3);
        control.set_maximum_width_request(-2);
        EXPECT_EQ(static_cast<const i_view&>(control).width(), 0);
        EXPECT_EQ(static_cast<const i_view&>(control).minimum_width(), 0);
        EXPECT_EQ(static_cast<const i_view&>(control).maximum_width(), 0);
    }

    TEST(size_request, minimum_unset_vs_explicit)
    {
        label control;
        EXPECT_TRUE(std::isnan(static_cast<const i_view&>(control).minimum_width())); // unset -> Unset
        control.set_minimum_width_request(30);
        EXPECT_EQ(static_cast<const i_view&>(control).minimum_width(), 30);
    }

    // ---- the per-child measure clamp (view<>::measure resolves content against the requests) ----

    TEST(size_request, measure_reports_content_when_unconstrained)
    {
        // With no size request set, measure passes the handler's content size straight through (no clamp,
        // no exact override). Compared against the handler's own GetDesiredSize so it is backend-agnostic
        // (the headless metric is ~7pt/char; AppKit's fittingSize differs but the pass-through must hold).
        label control;
        control.set_text("ABCDEFGHIJ");
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const size content = handler->get_desired_size(inf, inf);
        ASSERT_GT(content.width, 0); // the handler reports a real content width for non-empty text
        const size measured = control.measure(inf, inf);
        EXPECT_EQ(measured.width, content.width);
        EXPECT_EQ(measured.height, content.height);
    }

    TEST(size_request, measure_honors_explicit_request)
    {
        label control;
        control.set_text("ABCDEFGHIJ");
        control.set_width_request(120);
        control.set_height_request(50);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        const size measured = control.measure(inf, inf);
        EXPECT_EQ(measured.width, 120); // exact request overrides the 70 content
        EXPECT_EQ(measured.height, 50);
    }

    TEST(size_request, measure_caps_content_at_maximum)
    {
        label control;
        control.set_text("ABCDEFGHIJ"); // content width 70
        control.set_maximum_width_request(50);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        EXPECT_EQ(control.measure(inf, inf).width, 50); // 70 capped to 50
    }

    TEST(size_request, measure_floors_content_at_minimum)
    {
        label control;
        control.set_text("AB"); // content width 14
        control.set_minimum_width_request(100);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        EXPECT_EQ(control.measure(inf, inf).width, 100); // 14 floored to 100
    }

    TEST(size_request, measure_minimum_beats_maximum)
    {
        label control;
        control.set_text("ABCDEFGHIJ");
        control.set_minimum_width_request(80);
        control.set_maximum_width_request(50);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);

        EXPECT_EQ(control.measure(inf, inf).width, 80); // min wins over max
    }

    TEST(size_request, frame_is_independent_of_request)
    {
        // IView.Width reflects the REQUEST; the arranged frame is separate (set by Arrange via
        // VisualElement.ArrangeOverride -> LayoutExtensions.ComputeFrame). With an explicit WidthRequest the
        // frame consumes the (measured) desired size, NOT the raw arrange bounds: an explicit width wins over
        // the default Fill so the view is sized to 120 and CENTERED within the 200-wide bounds (C#'s
        // "width overrides fill from center"). The request reported by IView.Width is unchanged.
        label control;
        control.set_width_request(120);
        control.set_height_request(30);
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        control.measure(inf, inf); // populate DesiredSize (120 x 30 after the request clamp)

        control.arrange(maui::graphics::rect(0, 0, 200, 30));
        EXPECT_EQ(control.frame().width, 120); // sized to the request, not the 200 bounds
        EXPECT_EQ(control.frame().x, 40);      // (200 - 120) / 2 — fill-with-explicit-width centers
        EXPECT_EQ(static_cast<const i_view&>(control).width(), 120); // IView.Width still reports the request
    }
} // namespace
