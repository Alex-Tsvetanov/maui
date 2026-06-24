#pragma once
// maui::samples::custom_size_swipe_page — ports CustomSizeSwipeViewGallery.xaml (+ .xaml.cs)
//
// The MAUI CustomSizeSwipeViewGallery is a single SwipeView whose Left / Right / Top item collections each
// reveal CUSTOM-SIZED content: a SwipeItemView wrapping a Grid/StackLayout with an explicit
// WidthRequest=200 (left/right) or HeightRequest=100 (top), proving the swipe panel honors the item's own
// requested size rather than a fixed menu width. The right side ALSO carries a plain icon+text SwipeItem
// ("Test", red) before its custom SwipeItemView, so one side mixes a menu-style item with a custom-content
// item. The content itself is a green Grid holding a label + a button. The .xaml.cs wires three Clicked
// handlers (content button, right-items button, top-items button) that each pop a DisplayAlert — collapsed
// here to a readout label (no modal dialog seam headless).
//
// Self-contained (the swipe_refresh_page / value_controls_page pattern): the page OWNS its whole element
// tree, exposes page().
//
// Demonstrated (custom-size reveal panels + the three click channels):
//   - LeftItems:  one SwipeItemView -> a light-pink Grid (WidthRequest 200) with a centered label.
//   - RightItems: a red icon+text SwipeItem ("Test") THEN a SwipeItemView -> a light-steel-blue StackLayout
//                 (WidthRequest 200) with a label + "Test Click from RightItems" button.
//   - TopItems:   one SwipeItemView -> a light-sky-blue StackLayout (HeightRequest 100) with a label +
//                 "Click me!" button.
//   - Content:    a light-green Grid with a label + "Test Click from Content" button.
// Each of the three buttons' Clicked drives the readout (the C# DisplayAlert, collapsed).
//
// The page's mount hook finishes by synthetically OPENING the RightItems side (open(right_items)) so the
// static capture shows a revealed panel + reports the Threshold, then fires each button's Clicked so the
// readout reflects the wiring.

#include <cstdio>
#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class custom_size_swipe_page
    {
    public:
        custom_size_swipe_page()
        {
            page_.set_title("Custom Size SwipeView");

            readout_.set_text("Ready (swipe a side to reveal its custom-sized content)");

            // ---- LeftItems: one SwipeItemView -> light-pink Grid, WidthRequest 200 ----
            left_panel_.set_background(solid(maui::graphics::colors::light_pink));
            left_panel_.set_width_request(200);
            left_label_.set_text("This is the LeftItems Content");
            left_panel_.add(left_label_);
            left_item_view_.set_content(left_panel_);
            swipe_.left_items_collection().add(left_item_view_);

            // ---- RightItems: a red menu SwipeItem ("Test"), THEN a custom SwipeItemView (200 wide) ----
            right_menu_item_.set_text("Test"); // C# IconImageSource="calculator.png" — image source omitted headless
            right_menu_item_.set_background_color(maui::graphics::colors::red);
            swipe_.right_items_collection().add(right_menu_item_);

            right_panel_.set_background(solid(maui::graphics::colors::light_steel_blue));
            right_panel_.set_width_request(200);
            right_label_.set_text("This is the RightItems Content");
            right_button_.set_text("Test Click from RightItems");
            right_button_.clicked.connect([this] { readout_.set_text("RightItems button clicked"); });
            right_panel_.add(right_label_);
            right_panel_.add(right_button_);
            right_item_view_.set_content(right_panel_);
            swipe_.right_items_collection().add(right_item_view_);

            // ---- TopItems: one SwipeItemView -> light-sky-blue StackLayout, HeightRequest 100 ----
            top_panel_.set_background(solid(maui::graphics::colors::light_sky_blue));
            top_panel_.set_height_request(100);
            top_label_.set_text("This is the TopItems Content");
            top_button_.set_text("Click me!");
            top_button_.clicked.connect([this] { readout_.set_text("TopItems button clicked"); });
            top_panel_.add(top_label_);
            top_panel_.add(top_button_);
            top_item_view_.set_content(top_panel_);
            swipe_.top_items_collection().add(top_item_view_);

            // ---- Content: a green Grid with a label + a button ----
            content_panel_.set_background(solid(maui::graphics::colors::light_green));
            content_label_.set_text("This is the SwipeView Content");
            content_button_.set_text("Test Click from Content");
            content_button_.clicked.connect([this] { readout_.set_text("Content button clicked"); });
            content_stack_.add(content_label_);
            content_stack_.add(content_button_);
            content_panel_.add(content_stack_);
            swipe_.set_content(content_panel_);

            // The page hosts the SwipeView above a readout (the gallery convention; the C# page is the bare
            // SwipeView in a Grid — the readout stands in for the .xaml.cs DisplayAlert channel).
            root_.set_spacing(8);
            root_.add(swipe_);
            root_.add(readout_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Synthetically reveal the RightItems side so a static capture
        // shows the revealed panel, fire each wired button's Clicked, then report the open state + Threshold.
        // All per-control attach + re-host plumbing is now the generic mount's job.
        void on_mounted(maui::hosting::maui_app& /*app*/)
        {
            // Synthetically reveal one side (RightItems) so the capture shows the revealed SwipeItems, and
            // report the Threshold. SwipeView is interactive — open() is the developer-API seam the platform
            // would otherwise drive on a real swipe.
            swipe_.open(maui::core::open_swipe_item::right_items, /*animated=*/false);

            // Fire each button's Clicked so the readout reflects the three wired channels (send_clicked is
            // the handler-facing seam a native tap would invoke; the last call wins the readout).
            content_button_.send_clicked();
            top_button_.send_clicked();
            right_button_.send_clicked();

            char text[96];
            (void)std::snprintf(text, sizeof(text), "RightItems revealed (open=%d, threshold=%.0f)",
                                static_cast<int>(swipe_.is_open()), swipe_.threshold());
            readout_.set_text(text);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::swipe_view& swipe()
        {
            return swipe_;
        }
        [[nodiscard]] maui::controls::swipe_item& right_menu_item()
        {
            return right_menu_item_;
        }
        [[nodiscard]] maui::controls::button& content_button()
        {
            return content_button_;
        }
        [[nodiscard]] maui::controls::button& right_button()
        {
            return right_button_;
        }
        [[nodiscard]] maui::controls::button& top_button()
        {
            return top_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named BackgroundColor).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::swipe_view swipe_;
        maui::controls::label readout_;

        // LeftItems custom content.
        maui::controls::swipe_item_view left_item_view_;
        maui::controls::grid left_panel_;
        maui::controls::label left_label_;

        // RightItems: a menu item + custom content.
        maui::controls::swipe_item right_menu_item_; // owned: the swipe collection is non-owning
        maui::controls::swipe_item_view right_item_view_;
        maui::controls::vertical_stack_layout right_panel_;
        maui::controls::label right_label_;
        maui::controls::button right_button_;

        // TopItems custom content.
        maui::controls::swipe_item_view top_item_view_;
        maui::controls::vertical_stack_layout top_panel_;
        maui::controls::label top_label_;
        maui::controls::button top_button_;

        // SwipeView content.
        maui::controls::grid content_panel_;
        maui::controls::vertical_stack_layout content_stack_;
        maui::controls::label content_label_;
        maui::controls::button content_button_;
    };
} // namespace maui::samples
