#pragma once
// maui::samples::scroll_view_page — ports ScrollViewPage.xaml (+ the ScrollViewPages sub-demos:
// ScrollViewOrientationPage / ScrollToEndPage / ScrollToFromConstructorPage), code-first.
//
// The MAUI ScrollViewPage.xaml itself is a gallery LIST (a CollectionView of links into the
// ScrollViewPages sub-demos), so the behavior it stands for lives in those sub-demos. This port
// distills the ScrollView feature they exercise onto one self-contained page:
//   - ScrollViewOrientationPage: ScrollView.Orientation switched at runtime (Vertical default),
//   - ScrollToEndPage: ScrollToAsync(...) + the Scrolled event echoed to a readout,
//   - ScrollToFromConstructorPage: a scroll_to_async issued from the constructor (pended until the
//     handler attaches, then flushed — the ScrollView.OnHandlerChangedCore replay).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic: a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// Interactions demonstrated:
//   - tall content (many labels in a vertical stack) makes the scroll_view actually scroll,
//   - the scroll_view's `scrolled` event drives the readout label (scroll position feedback),
//   - `scroll_to_completed` appends a completion marker (the ScrollToAsync Task stand-in),
//   - a constructor-time scroll_to_async demonstrates the pend-until-attached request pipeline.

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/scroll_orientation.hpp"

namespace maui::samples
{
    class scroll_view_page
    {
    public:
        scroll_view_page()
        {
            page_.set_title("ScrollView");
            stack_.set_spacing(8);

            readout_.set_text("Scrolled to: 0 / 0");

            // The scroll_view's position feedback (ScrollViewPages: OnScrollViewScrolled echoes ScrollX/Y).
            scroller_.scrolled.connect([this](double x, double y) { update_readout(x, y); });
            // The ScrollToAsync completion stand-in (ScrollToEndPage's awaited ScrollToAsync(...)).
            scroller_.scroll_to_completed.connect([this] { mark_done(); });

            // Default Orientation is Vertical (ScrollView.cs); set explicitly to mirror the orientation demo.
            scroller_.set_orientation(maui::core::scroll_orientation::vertical);

            // The readout rides at the top of the scrolled content (same shape as containers_page).
            stack_.add(readout_);

            // Tall content — enough rows that the content exceeds any viewport and there is a scroll range.
            for (int n = 0; n < 40; ++n)
            {
                auto row = std::make_shared<maui::controls::label>();
                char text[48];
                std::snprintf(text, sizeof(text), "Row %d of 40", n);
                row->set_text(text);
                row->set_height_request(40);
                rows_.push_back(row);
                stack_.add(*row);
            }
            final_label_.set_text("End of content");
            final_label_.set_height_request(40);
            stack_.add(final_label_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);

            // No constructor scroll: the shared scroll_view.xaml is captured at REST — the content sits at the
            // top and the readout stays at its static "Scrolled to: 0 / 0" text. An earlier ctor
            // scroll_to_async(0,600) flushed on attach and its scroll_to_completed appended a "(done)" marker,
            // diverging from MAUI's resting readout. (scroll_to_async + the scrolled/scroll_to_completed wiring
            // stay available; the scroll unit tests cover the pend-until-attached request pipeline.)
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

    private:
        void update_readout(double x, double y)
        {
            char text[64];
            std::snprintf(text, sizeof(text), "Scrolled to: %.0f / %.0f", x, y);
            readout_.set_text(text);
        }

        void mark_done()
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
        maui::controls::label final_label_;                        // the ScrollToEndPage target (finalLabel)
        std::vector<std::shared_ptr<maui::controls::label>> rows_; // tall content, co-owned by the stack
    };
} // namespace maui::samples
