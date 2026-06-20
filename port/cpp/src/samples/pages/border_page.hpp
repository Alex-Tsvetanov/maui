#pragma once
// maui::samples::border_page — a faithful reproduction of the maui-compare "border" demo
// (ComparePages.BorderPage()), the shipped-.NET-MAUI reference for the visual-parity comparison: a single
// Border centered on the page — red 5pt stroke, a RoundRectangle StrokeShape (CornerRadius 20), a
// LightYellow BackgroundColor, Padding 16, a fixed WidthRequest 280 / HeightRequest 160, and
// HorizontalOptions/VerticalOptions Center — wrapping a Label ("Bordered content") centered both ways.
// Kept 1:1 with the C# reference.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class border_page
    {
    public:
        border_page()
        {
            page_.set_title("Border");

            // The centered Label content.
            caption_.set_text("Bordered content");
            caption_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            caption_.set_vertical_text_alignment(maui::core::text_alignment::center);

            // The Border: red 5pt stroke, RoundRectangle CornerRadius 20, LightYellow background, Padding 16,
            // a fixed 280x160, centered both ways on the page.
            bordered_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            bordered_.set_stroke_thickness(5);
            bordered_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(20.0));
            bordered_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_yellow));
            bordered_.set_padding(maui::core::thickness(16));
            bordered_.set_width_request(280);
            bordered_.set_height_request(160);
            bordered_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            bordered_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            bordered_.set_content(caption_);

            page_.set_content(bordered_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to the caption + the border + the page, then re-host the tree built in the ctor.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, caption_, "caption_");
            gallery_attach_one(app, bordered_, "bordered_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(bordered_); // the border hosts the caption
            gallery_rehost_content(page_);     // the page hosts the border
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::border& bordered()
        {
            return bordered_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::border bordered_;
        maui::controls::label caption_;
    };
} // namespace maui::samples
