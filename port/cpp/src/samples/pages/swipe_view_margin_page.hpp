#pragma once
// maui::samples::swipe_view_margin_page — ports SwipeViewMarginGallery.xaml
//
// A self-contained, code-first port of the .NET MAUI "SwipeView Margin Gallery": two swipe_views whose
// content's Margin + Padding are driven by two sliders, demonstrating that the revealed SwipeItems stay
// correctly positioned as the content insets change. One swipe_view has horizontal (Left/Right) items,
// the other vertical (Top/Bottom) items:
//
//   instructions label (black bg, white text)
//   "SwipeView Content Margin"  + MarginSlider  (0..48, value 12)
//   "SwipeView Content Padding" + PaddingSlider (0..48, value 12)
//   swipe_view #1 (LightGray): Left "Favourite" (green) + Right "Delete" (red), over a gray 100-tall grid
//   swipe_view #2 (LightGray): Top  "Favourite" (green) + Bottom "Delete" (red), over a gray 100-tall grid
//
// Interactions demonstrated:
//   - the PaddingSlider drives BOTH content grids' Padding (the MAUI Padding="{x:Reference PaddingSlider}"
//     binding, mirrored code-first via slider value_changed → grid.set_padding).
//   - the MarginSlider updates the readout with its value (see the note below).
//   - attach_handlers() synthetically OPENS swipe_view #1's left items so the static capture shows the
//     revealed Favourite item, and reports the open via open_requested → readout.
//
// The page OWNS its whole element tree; it is backend-agnostic. A sample main attaches handlers bottom-up
// and hosts page() in a window; the headless/apple/ios trees exercise the same wiring.
//
// note: MAUI's Margin="{x:Reference MarginSlider}" binding cannot be mirrored by a setter — the port's
// view::margin() is a read-only contract value (no settable Margin bindable at this layer), so the
// MarginSlider drives only the readout. The Padding binding IS settable (layout::set_padding) and is
// wired through. The MAUI slider track/thumb colors (LightGray/Gray/DarkGray) are applied where the port
// exposes them (minimum/maximum track + thumb color).

#include <cstdio>
#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
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
    class swipe_view_margin_page
    {
    public:
        swipe_view_margin_page()
        {
            page_.set_title("SwipeView Margin Gallery");
            stack_.set_padding(maui::core::thickness{12});

            readout_.set_text("Adjust the sliders, then open a row to verify item positioning");

            instructions_.set_text("Modify the SwipeView Margin and Padding values, and verify when opening "
                                   "that the positioning of the SwipeItems is correct.");
            instructions_.set_padding(maui::core::thickness{12});
            instructions_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black));
            instructions_.set_text_color(maui::graphics::colors::white);

            margin_caption_.set_text("SwipeView Content Margin");
            padding_caption_.set_text("SwipeView Content Padding");

            // ---- the two sliders (0..48, value 12; LightGray/Gray track + DarkGray thumb) ----
            configure_slider(margin_slider_);
            margin_slider_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { update_readout("Margin", new_value); });

            configure_slider(padding_slider_);
            padding_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                const maui::core::thickness inset{new_value};
                horizontal_grid_.set_padding(inset);
                vertical_grid_.set_padding(inset);
                update_readout("Padding", new_value);
            });

            // ---- swipe_view #1: horizontal (Left/Right) items over a gray 100-tall grid ----
            h_favourite_.set_text("Favourite");
            h_favourite_.set_background_color(maui::graphics::colors::green);
            h_delete_.set_text("Delete");
            h_delete_.set_background_color(maui::graphics::colors::red);

            horizontal_label_.set_text("Horizontal SwipeItems");
            horizontal_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            horizontal_label_.set_vertical_text_alignment(maui::core::text_alignment::center);
            horizontal_grid_.set_height_request(100);
            horizontal_grid_.set_padding(maui::core::thickness{12}); // the slider's initial value
            horizontal_grid_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::gray));
            horizontal_grid_.add(horizontal_label_);

            horizontal_swipe_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            horizontal_swipe_.left_items_collection().add(h_favourite_);
            horizontal_swipe_.right_items_collection().add(h_delete_);
            horizontal_swipe_.set_content(horizontal_grid_);
            horizontal_swipe_.open_requested.connect([this](const maui::core::swipe_view_open_request& /*r*/) {
                readout_.set_text("Horizontal items revealed");
            });

            // ---- swipe_view #2: vertical (Top/Bottom) items over a gray 100-tall grid ----
            v_favourite_.set_text("Favourite");
            v_favourite_.set_background_color(maui::graphics::colors::green);
            v_delete_.set_text("Delete");
            v_delete_.set_background_color(maui::graphics::colors::red);

            vertical_label_.set_text("Vertical SwipeItems");
            vertical_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            vertical_label_.set_vertical_text_alignment(maui::core::text_alignment::center);
            vertical_grid_.set_height_request(100);
            vertical_grid_.set_padding(maui::core::thickness{12});
            vertical_grid_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::gray));
            vertical_grid_.add(vertical_label_);

            vertical_swipe_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            vertical_swipe_.top_items_collection().add(v_favourite_);
            vertical_swipe_.bottom_items_collection().add(v_delete_);
            vertical_swipe_.set_content(vertical_grid_);

            stack_.set_spacing(6);
            stack_.add(readout_);
            stack_.add(instructions_);
            stack_.add(margin_caption_);
            stack_.add(margin_slider_);
            stack_.add(padding_caption_);
            stack_.add(padding_slider_);
            stack_.add(horizontal_swipe_);
            stack_.add(vertical_swipe_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP, then re-host the tree built in the ctor. The
        // four "Favourite"/"Delete" swipe_items are NON-view items (no standalone handler) and are
        // deliberately excluded; their content grids ARE views and are hosted (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, instructions_, "instructions_");
            gallery_attach_one(app, margin_caption_, "margin_caption_");
            gallery_attach_one(app, padding_caption_, "padding_caption_");
            gallery_attach_one(app, margin_slider_, "margin_slider_");
            gallery_attach_one(app, padding_slider_, "padding_slider_");

            gallery_attach_one(app, horizontal_label_, "horizontal_label_");
            gallery_attach_one(app, horizontal_grid_, "horizontal_grid_");
            gallery_attach_one(app, horizontal_swipe_, "horizontal_swipe_");
            gallery_attach_one(app, vertical_label_, "vertical_label_");
            gallery_attach_one(app, vertical_grid_, "vertical_grid_");
            gallery_attach_one(app, vertical_swipe_, "vertical_swipe_");

            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // Replay the host commands the ctor fired before any handler existed (bottom-up).
            gallery_rehost_layout(horizontal_grid_);   // grid hosts the centered label
            gallery_rehost_content(horizontal_swipe_); // swipe hosts the grid
            gallery_rehost_layout(vertical_grid_);
            gallery_rehost_content(vertical_swipe_);
            gallery_rehost_layout(stack_); // outer stack hosts captions + sliders + swipes
            gallery_rehost_content(page_); // page hosts the stack

            // Synthetically OPEN the horizontal swipe's left items so the static capture shows a revealed
            // Favourite, then push the initial padding through (the slider's default 12) so the readout and
            // the content insets agree.
            padding_slider_.set_value(12);
            horizontal_swipe_.open(maui::core::open_swipe_item::left_items);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::swipe_view& horizontal_swipe()
        {
            return horizontal_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& vertical_swipe()
        {
            return vertical_swipe_;
        }
        [[nodiscard]] maui::controls::slider& margin_slider()
        {
            return margin_slider_;
        }
        [[nodiscard]] maui::controls::slider& padding_slider()
        {
            return padding_slider_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        static void configure_slider(maui::controls::slider& slider)
        {
            slider.set_minimum(0);
            slider.set_maximum(48);
            slider.set_value(12);
            slider.set_minimum_track_color(maui::graphics::colors::light_gray);
            slider.set_maximum_track_color(maui::graphics::colors::gray);
            slider.set_thumb_color(maui::graphics::colors::dark_gray);
        }

        void update_readout(const char* which, double value)
        {
            char text[48];
            std::snprintf(text, sizeof(text), "%s = %.0f", which, value);
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::label instructions_;
        maui::controls::label margin_caption_;
        maui::controls::label padding_caption_;
        maui::controls::slider margin_slider_;
        maui::controls::slider padding_slider_;

        // swipe_view #1 — horizontal items.
        maui::controls::swipe_view horizontal_swipe_;
        maui::controls::grid horizontal_grid_;
        maui::controls::label horizontal_label_;
        maui::controls::swipe_item h_favourite_;
        maui::controls::swipe_item h_delete_;

        // swipe_view #2 — vertical items.
        maui::controls::swipe_view vertical_swipe_;
        maui::controls::grid vertical_grid_;
        maui::controls::label vertical_label_;
        maui::controls::swipe_item v_favourite_;
        maui::controls::swipe_item v_delete_;
    };
} // namespace maui::samples
