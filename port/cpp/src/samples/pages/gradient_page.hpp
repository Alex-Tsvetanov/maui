#pragma once
// maui::samples::gradient_page — a faithful reproduction of the maui-compare "gradient" demo
// (ComparePages.Gradient()), the shipped-.NET-MAUI reference for the visual-parity comparison: a
// VerticalStackLayout (Spacing 12, Padding 16) of two captioned 60px BoxViews — a LinearGradientBrush
// (Yellow→Green, horizontal: start (0,0) → end (1,0)) and a RadialGradientBrush (Red→DarkBlue, center
// (0.5,0.5), radius 0.7) — each under a bold caption. Kept 1:1 with the C# reference (the port renders
// Brush backgrounds through graphics::gradient_paint, so a LinearGradientBrush → linear_gradient_paint and
// a RadialGradientBrush → radial_gradient_paint with the same stops/geometry).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <memory>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class gradient_page
    {
    public:
        gradient_page()
        {
            page_.set_title("Gradient");
            stack_.set_spacing(12);
            stack_.set_padding(maui::core::thickness(16));

            const auto bold = maui::core::font::system_font_of_weight(maui::core::font_weight::bold);

            // LinearGradientBrush (Yellow→Green), horizontal left-to-right, on a 60px box.
            linear_caption_.set_text("LinearGradientBrush (yellow→green)");
            linear_caption_.set_font(bold);
            linear_box_.set_height_request(60);
            linear_box_.set_background(std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{
                    maui::graphics::gradient_stop(0.0F, maui::graphics::colors::yellow),
                    maui::graphics::gradient_stop(1.0F, maui::graphics::colors::green)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0)));

            // RadialGradientBrush (Red→DarkBlue), center (0.5,0.5), radius 0.7, on a 60px box.
            radial_caption_.set_text("RadialGradientBrush (red→navy)");
            radial_caption_.set_font(bold);
            radial_box_.set_height_request(60);
            radial_box_.set_background(std::make_shared<maui::graphics::radial_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{
                    maui::graphics::gradient_stop(0.0F, maui::graphics::colors::red),
                    maui::graphics::gradient_stop(1.0F, maui::graphics::colors::dark_blue)},
                maui::graphics::point(0.5, 0.5), 0.7));

            stack_.add(linear_caption_);
            stack_.add(linear_box_);
            stack_.add(radial_caption_);
            stack_.add(radial_box_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the captions + boxes, then the stack, then the
        // page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, linear_caption_, "linear_caption_");
            gallery_attach_one(app, linear_box_, "linear_box_");
            gallery_attach_one(app, radial_caption_, "radial_caption_");
            gallery_attach_one(app, radial_box_, "radial_box_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::box_view& linear_box()
        {
            return linear_box_;
        }
        [[nodiscard]] maui::controls::box_view& radial_box()
        {
            return radial_box_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label linear_caption_;
        maui::controls::box_view linear_box_;
        maui::controls::label radial_caption_;
        maui::controls::box_view radial_box_;
    };
} // namespace maui::samples
