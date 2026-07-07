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
//   - the MarginSlider drives BOTH content grids' Margin (mirroring the Padding wiring) and the readout.
//   - the page's mount hook synthetically OPENS swipe_view #1's left items so the static capture shows the
//     revealed Favourite item, and reports the open via open_requested → readout.
//
// The page OWNS its whole element tree; it is backend-agnostic. A sample main attaches handlers bottom-up
// and hosts page() in a window; the headless/apple/ios trees exercise the same wiring.
//
// note: MAUI's Margin="{x:Reference MarginSlider}" binding is mirrored code-first via the MarginSlider's
// value_changed → grid.set_margin (the View.Margin seam), exactly like the Padding binding (layout::
// set_padding). The MAUI slider track/thumb colors (LightGray/Gray/DarkGray) are applied where the port
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
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

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
            margin_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                const maui::core::thickness inset{new_value};
                horizontal_grid_.set_margin(inset);
                vertical_grid_.set_margin(inset);
                update_readout("Margin", new_value);
            });

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
            horizontal_grid_.set_padding(maui::core::thickness{12}); // the padding slider's initial value
            horizontal_grid_.set_margin(maui::core::thickness{12});  // the margin slider's initial value
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
            vertical_grid_.set_margin(maui::core::thickness{12});
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

        // No post-mount synthetic drive: the shared swipe_view_margin.xaml is captured at REST — both sliders
        // sit at their Value="12" (set in configure_slider BEFORE the value_changed handlers connect, so no
        // readout fire), the content grids are inset by 12, both SwipeViews are CLOSED, and the readout stays
        // at its static "Adjust the sliders, then open a row to verify item positioning" text. Synthetically
        // open()ing the horizontal swipe overwrote that readout ("Horizontal items revealed"), diverging from
        // MAUI. The slider value_changed handlers still drive the insets + readout interactively.

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
