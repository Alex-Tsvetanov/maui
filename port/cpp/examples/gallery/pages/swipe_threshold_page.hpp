#pragma once
// maui::samples::swipe_threshold_page — ports HorizontalSwipeThresholdGallery.xaml (+ .xaml.cs)
//
// The MAUI HorizontalSwipeThresholdGallery shows how SwipeView.Threshold (the swipe distance, in DIPs, the
// user must drag before the items settle open / execute) interacts with SwipeItems.Mode (Reveal vs
// Execute). Four right-swipe blocks, each a SwipeView over a dark-blue Grid with one orange SwipeItem:
//   1. Default Threshold, Reveal mode  — no Threshold set (the platform default).
//   2. Custom Threshold, Reveal mode   — a Slider (50..200, start 80) drives SwipeView.Threshold via a
//      Binding ({Binding Source=ThresholdRevealSlider, Path=Value}). C# wires Slider.ValueChanged ->
//      RevealThresholdSwipeView.Close() so the open panel snaps shut whenever the threshold changes.
//   3. Default Threshold, Execute mode — no Threshold set.
//   4. Custom Threshold, Execute mode  — a Slider (50..300, start 80) -> ExecuteThresholdSwipeView.Threshold,
//      its ValueChanged -> Close().
//
// Port mapping: the XAML {Binding ...Value} threshold + the .xaml.cs ValueChanged->Close() collapse into one
// slider value_changed handler — set_threshold(new_value) then close() — which is exactly what the two C#
// pieces do together. A note label carries the C# "Threshold is implemented on Android/iOS only" banner;
// the readout reports the live thresholds. SwipeItems.Mode is set via set_mode(reveal|execute).
//
// Self-contained (the swipe_refresh_page / value_controls_page pattern): the page OWNS its whole element
// tree, exposes page().
//
// The page's mount hook finishes by synthetically OPENING the two custom-threshold SwipeViews (so the static
// capture shows their revealed orange SwipeItems) and reports each live Threshold into the readout.

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
#include "maui/core/swipe_mode.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class swipe_threshold_page
    {
    public:
        swipe_threshold_page()
        {
            page_.set_title("Horizontal SwipeThreshold Gallery");
            root_.set_spacing(8);

            note_.set_text("The Threshold property is only implemented for now on Android and iOS.");
            note_.set_background(solid(maui::graphics::colors::black));
            note_.set_text_color(maui::graphics::colors::white);
            readout_.set_text("Ready");

            // ---- Block 1: Default Threshold (Reveal Mode) ----
            default_reveal_label_.set_text("Default Threshold (Reveal Mode)");
            build_block(default_reveal_swipe_, default_reveal_item_, default_reveal_content_,
                        maui::core::swipe_mode::reveal);

            // ---- Block 2: Custom Threshold (Reveal Mode) — a slider drives Threshold; change -> Close() ----
            custom_reveal_label_.set_text("Custom Threshold (only one SwipeItem using Reveal Mode)");
            reveal_slider_.set_minimum(50);
            reveal_slider_.set_maximum(200);
            reveal_slider_.set_value(80);
            reveal_slider_.set_maximum_track_color(maui::graphics::colors::gray);
            reveal_slider_.set_minimum_track_color(brand_blue());
            reveal_slider_.set_thumb_color(brand_blue());
            reveal_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                // The C# {Binding Value} -> Threshold + ValueChanged -> Close() collapsed into one step.
                custom_reveal_swipe_.set_threshold(new_value);
                custom_reveal_swipe_.close(/*animated=*/false);
                update_readout();
            });
            custom_reveal_swipe_.set_threshold(reveal_slider_.value()); // honor the slider's start value (80)
            build_block(custom_reveal_swipe_, custom_reveal_item_, custom_reveal_content_,
                        maui::core::swipe_mode::reveal);

            // ---- Block 3: Default Threshold (Execute Mode) ----
            default_execute_label_.set_text("Default Threshold (Execute Mode)");
            build_block(default_execute_swipe_, default_execute_item_, default_execute_content_,
                        maui::core::swipe_mode::execute);

            // ---- Block 4: Custom Threshold (Execute Mode) — slider 50..300 ----
            custom_execute_label_.set_text("Custom Threshold (only one SwipeItem using Execute Mode)");
            execute_slider_.set_minimum(50);
            execute_slider_.set_maximum(300);
            execute_slider_.set_value(80);
            execute_slider_.set_maximum_track_color(maui::graphics::colors::gray);
            execute_slider_.set_minimum_track_color(brand_blue());
            execute_slider_.set_thumb_color(brand_blue());
            execute_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                custom_execute_swipe_.set_threshold(new_value);
                custom_execute_swipe_.close(/*animated=*/false);
                update_readout();
            });
            custom_execute_swipe_.set_threshold(execute_slider_.value());
            build_block(custom_execute_swipe_, custom_execute_item_, custom_execute_content_,
                        maui::core::swipe_mode::execute);

            // Assemble the page in XAML order.
            root_.add(note_);
            root_.add(default_reveal_label_);
            root_.add(default_reveal_swipe_);
            root_.add(custom_reveal_label_);
            root_.add(reveal_slider_);
            root_.add(custom_reveal_swipe_);
            root_.add(default_execute_label_);
            root_.add(default_execute_swipe_);
            root_.add(custom_execute_label_);
            root_.add(execute_slider_);
            root_.add(custom_execute_swipe_);
            root_.add(readout_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Synthetically reveal the two custom-threshold SwipeViews so the
        // capture is non-blank, then report the live thresholds. All per-control attach + re-host plumbing is
        // now the generic mount's job.
        void on_mounted(maui::hosting::maui_app& /*app*/)
        {
            // open() is the developer-API seam the platform drives on a real swipe.
            custom_reveal_swipe_.open(maui::core::open_swipe_item::right_items, /*animated=*/false);
            custom_execute_swipe_.open(maui::core::open_swipe_item::right_items, /*animated=*/false);
            update_readout();
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::slider& reveal_slider()
        {
            return reveal_slider_;
        }
        [[nodiscard]] maui::controls::slider& execute_slider()
        {
            return execute_slider_;
        }
        [[nodiscard]] maui::controls::swipe_view& custom_reveal_swipe()
        {
            return custom_reveal_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& custom_execute_swipe()
        {
            return custom_execute_swipe_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // The XAML brand color #2E249E (BackgroundColor), shared by the Grid fills + the slider tracks.
        static maui::graphics::color brand_blue()
        {
            return maui::graphics::color::from_argb("#2E249E");
        }
        // The XAML SwipeItemBackgroundColor #FE744D (the orange SwipeItem fill).
        static maui::graphics::color swipe_item_orange()
        {
            return maui::graphics::color::from_argb("#FE744D");
        }
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        // Build one block: an orange SwipeItem (RightItems, the given Mode) + a dark-blue Grid content with
        // two labels. The SwipeView's HeightRequest=80 mirrors the XAML.
        void build_block(maui::controls::swipe_view& swipe, maui::controls::swipe_item& item,
                         maui::controls::grid& content, maui::core::swipe_mode mode)
        {
            swipe.set_height_request(80);
            item.set_text("SwipeItem"); // C# IconImageSource="calculator.png" — image source omitted headless
            item.set_background_color(swipe_item_orange());
            swipe.right_items_collection().set_mode(mode);
            swipe.right_items_collection().add(item);

            content.set_background(solid(brand_blue()));
            content.set_height_request(80);
            swipe.set_content(content);
        }

        void update_readout()
        {
            char text[96];
            (void)std::snprintf(text, sizeof(text), "Reveal threshold=%.0f / Execute threshold=%.0f",
                                custom_reveal_swipe_.threshold(), custom_execute_swipe_.threshold());
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label note_;
        maui::controls::label readout_;

        // Block 1: Default Reveal.
        maui::controls::label default_reveal_label_;
        maui::controls::swipe_view default_reveal_swipe_;
        maui::controls::swipe_item default_reveal_item_; // owned: the swipe collection is non-owning
        maui::controls::grid default_reveal_content_;

        // Block 2: Custom Reveal (slider-driven Threshold).
        maui::controls::label custom_reveal_label_;
        maui::controls::slider reveal_slider_;
        maui::controls::swipe_view custom_reveal_swipe_;
        maui::controls::swipe_item custom_reveal_item_;
        maui::controls::grid custom_reveal_content_;

        // Block 3: Default Execute.
        maui::controls::label default_execute_label_;
        maui::controls::swipe_view default_execute_swipe_;
        maui::controls::swipe_item default_execute_item_;
        maui::controls::grid default_execute_content_;

        // Block 4: Custom Execute (slider-driven Threshold).
        maui::controls::label custom_execute_label_;
        maui::controls::slider execute_slider_;
        maui::controls::swipe_view custom_execute_swipe_;
        maui::controls::swipe_item custom_execute_item_;
        maui::controls::grid custom_execute_content_;
    };
} // namespace maui::samples
