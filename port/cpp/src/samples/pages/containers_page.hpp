#pragma once
// containers_page — a self-contained demo page for the W1-07 container set: a scroll_view hosting a
// vertical stack of content-hosting containers — a border-framed label (stroke + dashed outline +
// rounded shape), a legacy frame (BorderColor/CornerRadius/HasShadow over the border machinery), and
// a content_view wrapping a label — with the scroll position echoed into a readout label (the C#
// gallery-page convention, code-first).
//
// The page OWNS its whole element tree (the sample_app pattern in maui_app_sample.mm). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts
// page() in a window; the headless/apple/ios test trees exercise the same wiring directly.
//
// Interactions demonstrated:
//   - the scroll_view's `scrolled` event drives the readout label (scroll position feedback),
//   - `scroll_to_completed` appends a completion marker (the ScrollToAsync Task stand-in),
//   - the border/frame/content_view sections show each container hosting live content.

#include <cstdio>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class containers_page
    {
    public:
        containers_page()
        {
            page_.set_title("Containers");
            stack_.set_spacing(12);

            readout_.set_text("Scrolled to: 0 / 0");
            scroller_.scrolled.connect([this](double x, double y) { update_readout(x, y); });
            scroller_.scroll_to_completed.connect([this] { readout_label_suffix(); });

            // border — a dashed, rounded stroke around a label.
            bordered_text_.set_text("Inside a border");
            border_.set_stroke(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color(0.16F, 0.50F, 0.73F)));
            border_.set_stroke_thickness(2);
            border_.set_stroke_dash_array({4.0, 2.0});
            border_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(8.0));
            border_.set_padding(maui::core::thickness(8));
            border_.set_content(bordered_text_);

            // frame — the legacy facade (BorderColor + CornerRadius + the canned shadow).
            framed_text_.set_text("Inside a frame");
            frame_.set_border_color(maui::graphics::color(0.86F, 0.20F, 0.27F));
            frame_.set_corner_radius(6.0F);
            frame_.set_content(framed_text_);

            // content_view — the simple single-content wrapper.
            wrapped_text_ = std::make_shared<maui::controls::label>();
            wrapped_text_->set_text("Inside a content_view");
            wrapper_.set_content(wrapped_text_);

            stack_.add(readout_);
            stack_.add(border_);
            stack_.add(frame_);
            stack_.add(wrapper_);
            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::border& bordered()
        {
            return border_;
        }
        [[nodiscard]] maui::controls::label& bordered_text()
        {
            return bordered_text_;
        }
        [[nodiscard]] maui::controls::frame& framed()
        {
            return frame_;
        }
        [[nodiscard]] maui::controls::label& framed_text()
        {
            return framed_text_;
        }
        [[nodiscard]] maui::controls::content_view& wrapper()
        {
            return wrapper_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::label>& wrapped_text()
        {
            return wrapped_text_;
        }

    private:
        void update_readout(double x, double y)
        {
            char text[64];
            std::snprintf(text, sizeof(text), "Scrolled to: %.0f / %.0f", x, y);
            readout_.set_text(text);
        }

        void readout_label_suffix()
        {
            char text[80];
            std::snprintf(text, sizeof(text), "Scrolled to: %.0f / %.0f (done)", scroller_.scroll_x(),
                          scroller_.scroll_y());
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::border border_;
        maui::controls::label bordered_text_;
        maui::controls::frame frame_;
        maui::controls::label framed_text_;
        maui::controls::content_view wrapper_;
        std::shared_ptr<maui::controls::label> wrapped_text_; // content_view co-owns its content
    };
} // namespace maui::samples
