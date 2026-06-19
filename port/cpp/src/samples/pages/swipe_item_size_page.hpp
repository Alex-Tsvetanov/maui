#pragma once
// maui::samples::swipe_item_size_page — ports SwipeItemSizeGallery.xaml
//
// A self-contained, code-first port of the .NET MAUI "SwipeItem Size Gallery": a scrolling stack of
// swipe_views demonstrating how a left SwipeItem's icon size and the SwipeView content size interact.
// Two groups, each a labeled set of swipe_views with a left "Delete" SwipeItem (red, an icon source,
// Invoked→OnSwipeItemInvoked) over a LightGray content:
//
//   Group 1 "Different icon sizes" — three swipe_views over a 60-tall gray grid, the icon source at
//     128 / 256 / 512 px (MAUI uses three remote flaticon URLs at those resolutions).
//   Group 2 "Different SwipeView sizes" — four swipe_views (all icon 512) over content of height
//     128 / 256 / 512 / unset (the last over a StackLayout with two labels, no explicit size).
//
// Interactions demonstrated:
//   - every left SwipeItem's Invoked event (the MAUI OnSwipeItemInvoked → DisplayAlert) is wired to the
//     readout: invoking the Delete item sets "Delete SwipeItem Invoked".
//   - attach_handlers() synthetically OPENS the first swipe_view's left items (open(left_items)) so the
//     static capture shows a revealed Delete item, and reports the open via open_requested → readout.
//
// The page OWNS its whole element tree; it is backend-agnostic. A sample main attaches handlers bottom-up
// and hosts page() in a window; the headless/apple/ios trees exercise the same wiring.
//
// note: MAUI's icon sources are remote https flaticon URLs; the port mints uri_image_source via
// image_source::from_uri(...) (the same URLs) — headless/CI never fetches them (an async no-op), so the
// item still reveals with its text + red background. The MAUI OnSwipeItemInvoked DisplayAlert (a modal)
// is replaced by the in-page readout, the observable swipe interaction.

#include <array>
#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp" // image_source::from_uri
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class swipe_item_size_page
    {
    public:
        // One "Delete" left-item + its swipe_view + the content grid/stack the swipe wraps.
        struct sized_row
        {
            maui::controls::swipe_view swipe;
            maui::controls::swipe_item delete_item;
            maui::controls::grid content_grid;                   // used when the row has an explicit-height grid
            maui::controls::vertical_stack_layout content_stack; // used only by the "No Size" row
            maui::controls::label content_label;
            maui::controls::label content_label2; // only the "No Size" row uses a second label
            bool uses_stack = false;
        };

        swipe_item_size_page()
        {
            page_.set_title("SwipeItem Size Gallery");
            stack_.set_padding(maui::core::thickness{12});

            readout_.set_text("Swipe a row left to reveal Delete");

            icon_sizes_header_.set_text("Different icon sizes");
            icon_sizes_header_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));

            view_sizes_header_.set_text("Different SwipeView sizes");
            view_sizes_header_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));

            stack_.add(readout_);
            stack_.add(icon_sizes_header_);

            // Group 1 — three rows over a 60-tall grid, icon at 128 / 256 / 512.
            configure_grid_row(rows_[0], "128x128 Icon", "https://image.flaticon.com/icons/png/128/61/61848.png", 60);
            configure_grid_row(rows_[1], "256x256 Icon", "https://image.flaticon.com/icons/png/256/61/61848.png", 60);
            configure_grid_row(rows_[2], "512x512 Icon", "https://image.flaticon.com/icons/png/512/61/61848.png", 60);
            stack_.add(group1_labels_[0]);
            stack_.add(rows_[0].swipe);
            stack_.add(group1_labels_[1]);
            stack_.add(rows_[1].swipe);
            stack_.add(group1_labels_[2]);
            stack_.add(rows_[2].swipe);

            stack_.add(view_sizes_header_);

            // Group 2 — four rows (icon 512) over content of height 128 / 256 / 512 / unset.
            configure_grid_row(rows_[3], "SwipeView 128 Height",
                               "https://image.flaticon.com/icons/png/512/61/61848.png", 128);
            configure_grid_row(rows_[4], "SwipeView 256 Height",
                               "https://image.flaticon.com/icons/png/512/61/61848.png", 256);
            configure_grid_row(rows_[5], "SwipeView 512 Height",
                               "https://image.flaticon.com/icons/png/512/61/61848.png", 512);
            configure_stack_row(rows_[6], "SwipeView No Size", "https://image.flaticon.com/icons/png/512/61/61848.png");
            stack_.add(group2_labels_[0]);
            stack_.add(rows_[3].swipe);
            stack_.add(group2_labels_[1]);
            stack_.add(rows_[4].swipe);
            stack_.add(group2_labels_[2]);
            stack_.add(rows_[5].swipe);
            stack_.add(group2_labels_[3]);
            stack_.add(rows_[6].swipe);

            // Group label texts (kept as owned labels so they have stable handlers).
            group1_labels_[0].set_text("128x128 Icon");
            group1_labels_[1].set_text("256x256 Icon");
            group1_labels_[2].set_text("512x512 Icon");
            group2_labels_[0].set_text("SwipeView 128 Height");
            group2_labels_[1].set_text("SwipeView 256 Height");
            group2_labels_[2].set_text("SwipeView 512 Height");
            group2_labels_[3].set_text("SwipeView No Size");

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP, then re-host the tree built in the ctor. Each
        // swipe_view's left "Delete" swipe_item is a NON-view item (no standalone handler), so it is
        // deliberately excluded from attach (attaching it would throw) — but its content grid/stack IS a
        // view and is hosted (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, icon_sizes_header_, "icon_sizes_header_");
            gallery_attach_one(app, view_sizes_header_, "view_sizes_header_");
            for (auto& label : group1_labels_)
            {
                gallery_attach_one(app, label, "group1_label");
            }
            for (auto& label : group2_labels_)
            {
                gallery_attach_one(app, label, "group2_label");
            }

            for (auto& row : rows_)
            {
                gallery_attach_one(app, row.content_label, "content_label");
                if (row.uses_stack)
                {
                    gallery_attach_one(app, row.content_label2, "content_label2");
                    gallery_attach_one(app, row.content_stack, "content_stack");
                }
                else
                {
                    gallery_attach_one(app, row.content_grid, "content_grid");
                }
                gallery_attach_one(app, row.swipe, "row.swipe");
            }

            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroll_, "scroll_");
            gallery_attach_one(app, page_, "page_");

            // Replay the host commands the ctor fired before any handler existed (bottom-up).
            for (auto& row : rows_)
            {
                if (row.uses_stack)
                {
                    gallery_rehost_layout(row.content_stack); // stack hosts its two labels
                }
                else
                {
                    gallery_rehost_layout(row.content_grid); // grid hosts the centered label
                }
                gallery_rehost_content(row.swipe); // swipe_view hosts its content
            }
            gallery_rehost_layout(stack_);   // outer stack hosts headers + rows + readout
            gallery_rehost_content(scroll_); // scroll hosts the stack
            gallery_rehost_content(page_);   // page hosts the scroll

            // Synthetically OPEN the first row's left items so the static capture shows a revealed Delete.
            rows_[0].swipe.open(maui::core::open_swipe_item::left_items);
        }

        [[nodiscard]] std::array<sized_row, 7>& rows()
        {
            return rows_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // Build a row whose content is an explicit-height gray grid with one centered label.
        void configure_grid_row(sized_row& row, const std::string& content_text, const std::string& icon_uri,
                                double content_height)
        {
            configure_delete_item(row, icon_uri);
            (void)content_text; // the per-row group label carries the heading; the content text is fixed

            row.content_label.set_text("Swipe to Left");
            row.content_label.set_horizontal_text_alignment(maui::core::text_alignment::center);
            row.content_label.set_vertical_text_alignment(maui::core::text_alignment::center);

            row.content_grid.set_height_request(content_height);
            row.content_grid.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            row.content_grid.add(row.content_label);

            row.swipe.left_items_collection().add(row.delete_item);
            row.swipe.set_content(row.content_grid);
        }

        // Build the "No Size" row: a LightGray StackLayout (no explicit height) with two labels.
        void configure_stack_row(sized_row& row, const std::string& /*unused*/, const std::string& icon_uri)
        {
            configure_delete_item(row, icon_uri);
            row.uses_stack = true;

            row.content_label.set_text("Swipe to Left");
            row.content_label2.set_text("SwipeView without Size");
            row.content_label2.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));

            row.content_stack.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            row.content_stack.add(row.content_label);
            row.content_stack.add(row.content_label2);

            row.swipe.left_items_collection().add(row.delete_item);
            row.swipe.set_content(row.content_stack);
        }

        // The shared left "Delete" SwipeItem: red, an icon source, Invoked→readout (the OnSwipeItemInvoked
        // handler MAUI shares across all the SwipeItems in this gallery).
        void configure_delete_item(sized_row& row, const std::string& icon_uri)
        {
            row.delete_item.set_text("Delete");
            row.delete_item.set_background_color(maui::graphics::colors::red);
            row.delete_item.set_icon_image_source(maui::controls::image_source::from_uri(icon_uri));
            row.delete_item.invoked.connect([this] { readout_.set_text("Delete SwipeItem Invoked"); });
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::label icon_sizes_header_;
        maui::controls::label view_sizes_header_;
        std::array<maui::controls::label, 3> group1_labels_;
        std::array<maui::controls::label, 4> group2_labels_;
        std::array<sized_row, 7> rows_;
    };
} // namespace maui::samples
