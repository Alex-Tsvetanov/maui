#pragma once
// maui::samples::clipping_page — ports ClippingPage.xaml (+ ClippingPage.xaml.cs)
//
// The C# page demonstrates Layout.IsClippedToBounds: a "Toggle clipping" button flips
// IsClippedToBounds on three horizontal stack layouts (an 8-button overflow row, a width-constrained
// purple-button row, and an image row whose second image is pushed down by a top margin). A Status
// label echoes "Clipping" / "Not clipping".
//
// PORT MAPPING:
//   - Layout.IsClippedToBounds  -> layout::set_clips_to_bounds / clips_to_bounds (controls/layout.hpp).
//     The button's `command` flips it on all three rows and rewrites the status label (the C#
//     ToggleClip.Clicked handler, including the Status.Text = Layout1.IsClippedToBounds ? … ternary).
//   - StackLayout Orientation="Horizontal"  -> stack_layout with stack_orientation::horizontal
//     (controls/stack_layout.hpp) — the generic stack so the single XAML <StackLayout> ports 1:1.
//   - the two <HorizontalStackLayout> rows  -> horizontal_stack_layout (the fixed-orientation control).
//
// ADDED (the prompt's "Clip with an i_shape"): the C# IsClippedToBounds toggle is the *box* clip; the
// richer geometry Clip (VisualElement.Clip / IView.Clip, an IShape) is a distinct feature. To exercise
// it faithfully the toggle ALSO sets/clears a rounded-rectangle clip geometry on the first row via
// view::set_clip (controls/view.hpp clip_property → graphics::shapes::round_rectangle), so the page
// demonstrates BOTH clip surfaces the way the framework models them.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree and attaches every owned view
// bottom-up (the value_controls_page / shapes_page convention).
//
// note: the BoxView Opacity="0.5" and the per-button/-image Margin from the XAML are layout polish;
//       the headless view surface exposes set_width_request/set_height_request (applied below) but no
//       per-view Margin or Opacity setter, so those decorative attributes are left as best-effort.

#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class clipping_page
    {
    public:
        clipping_page()
        {
            page_.set_title("Clipping");
            // Background="Orange" (ClippingPage.xaml root) — Orange = #FFA500.
            page_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_rgb(255, 165, 0)));

            root_.set_spacing(5);

            status_.set_text("Not clipping"); // Label x:Name="Status".

            toggle_clip_.set_text("Toggle clipping on horizontal stack layouts");
            // The ClippingPage.xaml.cs ToggleClip.Clicked handler: flip IsClippedToBounds on all three
            // rows, then set Status from Layout1's new state.
            toggle_clip_.command = [this] { on_toggle_clip(); };

            // ---- Layout1: the 8-button overflow row (StackLayout Orientation="Horizontal") ----
            row1_.set_orientation(maui::controls::stack_orientation::horizontal);
            for (int i = 0; i < kRow1Buttons; ++i)
            {
                auto button = std::make_shared<maui::controls::button>();
                button->set_text(std::to_string(i + 1));
                button->set_height_request(50);
                row1_.add(*button);
                row1_buttons_.push_back(std::move(button));
            }
            // BoxView Grid.Column="1" Background="Red" Opacity="0.5" — the overlay the unclipped row
            // spills over. Rendered as a plain red block (opacity is best-effort; see header note).
            overlay_.set_color(maui::graphics::color::from_rgb(255, 0, 0));

            // ---- Layout2: the width-constrained purple-button row (HorizontalStackLayout) ----
            row2_.set_width_request(100);
            for (int i = 0; i < kRow2Buttons; ++i)
            {
                auto button = std::make_shared<maui::controls::button>();
                button->set_text("Hey");
                button->set_width_request(50);
                button->set_height_request(50);
                button->set_background(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_rgb(128, 0, 128)));
                row2_.add(*button);
                row2_buttons_.push_back(std::move(button));
            }

            // ---- Layout3: the coffee-image row (HorizontalStackLayout) ----
            row3_.set_height_request(30);
            for (int i = 0; i < kRow3Images; ++i)
            {
                auto picture = std::make_shared<maui::controls::image>();
                picture->set_width_request(50);
                picture->set_height_request(50);
                row3_.add(*picture);
                row3_images_.push_back(std::move(picture));
            }

            root_.add(status_);
            root_.add(toggle_clip_);
            root_.add(row1_);
            root_.add(overlay_);
            root_.add(row2_);
            root_.add(row3_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) { gallery_attach_one(app, view, name); };

            one(status_, "status_");
            one(toggle_clip_, "toggle_clip_");
            for (const auto& button : row1_buttons_)
            {
                one(*button, "row1_button");
            }
            one(row1_, "row1_");
            one(overlay_, "overlay_");
            for (const auto& button : row2_buttons_)
            {
                one(*button, "row2_button");
            }
            one(row2_, "row2_");
            for (const auto& picture : row3_images_)
            {
                one(*picture, "row3_image");
            }
            one(row3_, "row3_");
            one(root_, "root_");
            one(page_, "page_");

            gallery_rehost_layout(row1_);
            gallery_rehost_layout(row2_);
            gallery_rehost_layout(row3_);
            gallery_rehost_layout(root_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::button& toggle_clip()
        {
            return toggle_clip_;
        }
        [[nodiscard]] maui::controls::stack_layout& row1()
        {
            return row1_;
        }
        [[nodiscard]] maui::controls::horizontal_stack_layout& row2()
        {
            return row2_;
        }
        [[nodiscard]] maui::controls::horizontal_stack_layout& row3()
        {
            return row3_;
        }

    private:
        // The C# ToggleClip.Clicked handler: flip IsClippedToBounds on the three rows, then echo the
        // first row's new state into the status label. The geometry-clip extension (see header) sets a
        // rounded-rectangle Clip on row1 while clipping is ON, and clears it when OFF.
        void on_toggle_clip()
        {
            const bool clipping = !row1_.clips_to_bounds();
            row1_.set_clips_to_bounds(clipping);
            row2_.set_clips_to_bounds(clipping);
            row3_.set_clips_to_bounds(clipping);

            // The geometry Clip (IView.Clip) — a rounded rect over the row's bounds while clipping is on.
            row1_.set_clip(clipping ? std::make_shared<maui::graphics::shapes::round_rectangle>(8.0) : nullptr);

            status_.set_text(clipping ? "Clipping" : "Not clipping");
        }

        static constexpr int kRow1Buttons = 8;
        static constexpr int kRow2Buttons = 4;
        static constexpr int kRow3Images = 2;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label status_;
        maui::controls::button toggle_clip_;
        maui::controls::stack_layout row1_;            // StackLayout Orientation="Horizontal" (Layout1)
        maui::controls::box_view overlay_;             // the translucent red BoxView the row spills onto
        maui::controls::horizontal_stack_layout row2_; // Layout2
        maui::controls::horizontal_stack_layout row3_; // Layout3
        std::vector<std::shared_ptr<maui::controls::button>> row1_buttons_;
        std::vector<std::shared_ptr<maui::controls::button>> row2_buttons_;
        std::vector<std::shared_ptr<maui::controls::image>> row3_images_;
    };
} // namespace maui::samples
