#pragma once
// swipe_refresh_page — a self-contained demo page for the W2-20 swipe + refresh controls: a refresh_view
// wrapping a swipe_view (which itself wraps a labeled row), with a readout label reflecting the latest
// interaction. Code-first, the C# gallery-page convention.
//
// The page OWNS its whole element tree. It is backend-agnostic — a sample main attaches handlers
// bottom-up and hosts page() in a window; the headless/apple/ios test trees exercise the same wiring.
//
// Interactions demonstrated:
//   - the swipe_view's left swipe item (Execute mode) raises Invoked → the readout shows "Deleted",
//   - the swipe_view's SwipeEnded updates the readout with the swipe direction + open state,
//   - the refresh_view's Refreshing event bumps a refresh counter into the readout, and its Command
//     (the W1-11 ICommand collapse) runs alongside.

#include <cstdio>
#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class swipe_refresh_page
    {
    public:
        swipe_refresh_page()
        {
            page_.set_title("Swipe + Refresh");

            row_.set_text("Swipe left to delete, pull to refresh");
            readout_.set_text("Ready");

            // The swipe item (Execute mode: a swipe past the threshold runs it immediately).
            delete_item_.set_text("Delete");
            delete_item_.set_background_color(maui::graphics::color(0.86F, 0.20F, 0.27F));
            delete_item_.invoked.connect([this] { readout_.set_text("Deleted"); });
            swipe_.right_items_collection().set_mode(maui::core::swipe_mode::execute);
            swipe_.right_items_collection().add(delete_item_);
            swipe_.set_content(row_);
            swipe_.swipe_ended.connect([this](const maui::core::swipe_view_swipe_ended& args) {
                char text[64];
                std::snprintf(text, sizeof(text), "Swipe ended (open=%d)", static_cast<int>(args.is_open));
                readout_.set_text(text);
            });

            // The refresh host wraps the swipe row.
            refresh_.set_command([this] {
                ++refresh_count_;
                char text[48];
                std::snprintf(text, sizeof(text), "Refreshed x%d", refresh_count_);
                readout_.set_text(text);
                refresh_.set_is_refreshing(false); // end the spinner (the gallery convention)
            });
            refresh_.set_content(swipe_);

            page_.set_content(refresh_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::refresh_view& refresh()
        {
            return refresh_;
        }
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item& delete_item()
        {
            return delete_item_;
        }
        [[nodiscard]] maui::controls::label& row()
        {
            return row_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] int refresh_count() const
        {
            return refresh_count_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::refresh_view refresh_;
        maui::controls::swipe_view swipe_;
        maui::controls::swipe_item delete_item_; // owned: the swipe collection is non-owning
        maui::controls::label row_;
        maui::controls::label readout_;
        int refresh_count_ = 0;
    };
} // namespace maui::samples
