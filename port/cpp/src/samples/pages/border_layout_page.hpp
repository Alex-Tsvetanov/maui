#pragma once
// maui::samples::border_layout_page — ports BorderLayout.xaml (+ BorderLayout.xaml.cs)
//
// The C# page demonstrates driving Border.StrokeThickness from a Slider: a Slider (0..40, set to 5 in
// OnAppearing) is bound to the Border's StrokeThickness; the Border (Silver stroke, White background,
// RoundRectangle CornerRadius 30) wraps a green Grid whose horizontal StackLayout holds a red Frame
// square, a centered "Center" Label, and a blue Frame square. As the slider moves, the rounded silver
// stroke around the green content thickens / thins.
//
// PORT MAPPING:
//   - the XAML {Binding Value, Source={x:Reference BorderWidthSlider}} → StrokeThickness is reproduced
//     imperatively: slider_.value_changed drives border_.set_stroke_thickness(new_value) and the readout
//     (the port's headless-safe equivalent of the one-way binding; the value engine flows the change to
//     the border_handler exactly as the binding would).
//   - Border.Stroke="Silver" / Background="White" → solid_paint over the named colors; the
//     RoundRectangle CornerRadius=30 → graphics::shapes::round_rectangle(30) on set_stroke_shape.
//   - the two colored <Frame> squares (Padding 0, HasShadow false, fixed 40x40) are plain colored
//     blocks → box_view (set_color + size requests), the same stand-in clipping_page uses for a
//     decorative colored rectangle (a Frame here carries no content, only a background color + size).
//   - the green <Grid HeightRequest=30> hosting the horizontal <StackLayout> → grid + a
//     horizontal_stack_layout child; the Label is centered text.
//
// note: BorderLayout.xaml.cs sets BorderWidthSlider.Value = 5 in OnAppearing; the port has no OnAppearing
//       hook on this code-first page, so the ctor seeds the slider to 5 directly (same observable start
//       state — a 5pt silver stroke).
// note: the per-view VerticalOptions="Center" / HorizontalOptions="Fill"+text-alignment from the XAML are
//       layout polish; the M2 view surface hardcodes layout alignment to Fill and exposes no
//       HorizontalOptions / VerticalOptions setter yet, so those decorative attributes are best-effort
//       (the content is reproduced faithfully; alignment is deferred). The Slider Maximum=40 cap and the
//       0-margin/0-padding on the Border are honored.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree and re-hosts it bottom-up via the
// shared gallery_attach helpers (Border is a single-content host → gallery_rehost_content).

#include <cstdio>

#include "maui/controls/border.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class border_layout_page
    {
    public:
        border_layout_page()
        {
            page_.set_title("Border using Layouts");
            stack_.set_spacing(12);

            // Slider (Minimum=0, Maximum=40) — the StrokeThickness driver. OnAppearing seeds Value=5.
            slider_.set_minimum(0);
            slider_.set_maximum(40);
            slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                border_.set_stroke_thickness(new_value);
                update_readout(new_value);
            });

            // Border: Silver stroke, White background, RoundRectangle CornerRadius=30, Margin/Padding 0.
            border_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::silver));
            border_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::white));
            border_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(30.0));
            border_.set_padding(maui::core::thickness(0));
            border_.set_stroke_thickness(5); // OnAppearing's slider start value → a 5pt silver stroke

            // Grid (BackgroundColor=Green, HeightRequest=30) hosting the horizontal content row.
            content_grid_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green));
            content_grid_.set_height_request(30);

            // The horizontal StackLayout: red square, centered "Center" label, blue square.
            // The two <Frame> squares carry no content — only a fill color + a fixed 40x40 size → box_view.
            left_square_.set_color(maui::graphics::colors::red);
            left_square_.set_width_request(40);
            left_square_.set_height_request(40);

            // Match the oracle: FontSize=17 (Medium), Horizontal/VerticalTextAlignment=Center, and
            // VerticalOptions=Center so the short label sits vertically centered in the 40pt row (the prior
            // "alignment deferred" note is stale — the view surface now exposes these setters).
            center_label_.set_text("Center");
            center_label_.set_font(maui::core::font::system_font_of_size(17.0));
            center_label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            center_label_.set_vertical_text_alignment(maui::core::text_alignment::center);
            center_label_.set_vertical_layout_alignment(maui::core::layout_alignment::center);

            right_square_.set_color(maui::graphics::colors::blue);
            right_square_.set_width_request(40);
            right_square_.set_height_request(40);

            row_.add(left_square_);
            row_.add(center_label_);
            row_.add(right_square_);
            content_grid_.add(row_);
            border_.set_content(content_grid_);

            readout_.set_text("Stroke thickness: 5 / 40");

            stack_.add(readout_);
            stack_.add(slider_);
            stack_.add(border_);
            page_.set_content(stack_);

            slider_.set_value(5); // fire the wiring once so the border + readout reflect the start state
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp). The border is a single-content host (rehost_content);
        // the grid + the horizontal row + the outer stack are layouts (rehost_layout).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, slider_, "slider_");
            gallery_attach_one(app, left_square_, "left_square_");
            gallery_attach_one(app, center_label_, "center_label_");
            gallery_attach_one(app, right_square_, "right_square_");
            gallery_attach_one(app, row_, "row_");
            gallery_attach_one(app, content_grid_, "content_grid_");
            gallery_attach_one(app, border_, "border_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(row_);          // horizontal stack hosts the two squares + the label
            gallery_rehost_layout(content_grid_); // grid hosts the row
            gallery_rehost_content(border_);      // border hosts the grid
            gallery_rehost_layout(stack_);        // outer stack hosts readout + slider + border
            gallery_rehost_content(page_);        // page hosts the stack
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::slider& width_slider()
        {
            return slider_;
        }
        [[nodiscard]] maui::controls::border& bordered()
        {
            return border_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        void update_readout(double thickness)
        {
            char text[48];
            std::snprintf(text, sizeof(text), "Stroke thickness: %.0f / 40", thickness);
            readout_.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label readout_;
        maui::controls::slider slider_;
        maui::controls::border border_;
        maui::controls::grid content_grid_;
        maui::controls::horizontal_stack_layout row_;
        maui::controls::box_view left_square_;
        maui::controls::label center_label_;
        maui::controls::box_view right_square_;
    };
} // namespace maui::samples
