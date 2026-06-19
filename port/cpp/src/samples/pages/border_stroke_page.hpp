#pragma once
// maui::samples::border_stroke_page — ports BorderStroke.xaml (+ BorderStroke.xaml.cs)
//
// A self-contained, code-first demo of Border StrokeThickness and how a Border tracks the height of its
// content. It mirrors the C# gallery page (Pages/Core/BorderGalleries/BorderStroke.xaml): a ScrollView over
// a margined VerticalStackLayout with two captioned sections, each a 3-row Grid of rectangle-shaped Borders
// wrapping an orange, centered Label —
//   - "Using different StrokeThickness": three Rectangle-shape Borders (red stroke, thickness 1 / 5 / 10),
//     each over an orange "1"/"5"/"10" label of minimum height 20; and
//   - "Updating the Content Height": a Slider (Minimum 40, Maximum 100) plus three more Rectangle-shape
//     Borders (red stroke, thickness 1 / 5 / 10) whose orange labels bind their HeightRequest to the slider
//     Value — so dragging the slider grows every content label and the borders track it.
//
// The C# code-behind sets ContentHeightSlider.Value = 60 in OnAppearing (the seed height). This port
// reproduces that by seeding the slider at 60 and pushing its value into the three height-tracked labels
// via the slider's value_changed event (the {Binding Value, Source=ContentHeightSlider} HeightRequest
// bindings), plus a readout caption echoing the live height — the same observable effect without the XAML
// element-source binding (a deferred layer-6 facility).
//
// The page OWNS its whole element tree (the gallery_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# <Style TargetType="Border"> resource (Margin="0,6" on every Border) is a margin facility
//         the port's view base does not expose; omitted with this note (the same note grid_page.hpp records
//         for layout options) — StrokeThickness/StrokeShape/Stroke map 1:1.
//   note: the C# Grid RowDefinitions="*,*,*" lays the three borders in three star rows; reproduced as a
//         3-star-row grid with each border placed via set_row (the grid_page.hpp cell-placement pattern).
//   note: each border's StrokeShape="Rectangle" is the rectangle clip shape
//         (maui::graphics::shapes::rectangle — an i_shape), the documented Border StrokeShape geometry.
//   note: the labels' BackgroundColor="Orange" is applied via set_background(solid_paint(orange)) — the
//         view base exposes the background as a paint, not a bare color (view.hpp); MinimumHeightRequest
//         maps to set_minimum_height_request where present, else the height seed carries the visual intent.
//   note: the second section's HeightRequest="{Binding Value, Source=ContentHeightSlider}" element-source
//         bindings are reproduced imperatively: the slider's value_changed drives each tracked label's
//         height_request (the headless-safe equivalent of the deferred XAML element binding).

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/grid_unit_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class border_stroke_page
    {
    public:
        border_stroke_page()
        {
            page_.set_title("Border Stroke options");
            stack_.set_spacing(12);

            // --- Section 1: "Using different StrokeThickness" (fixed-height content).
            fixed_caption_.set_text("Using different StrokeThickness");
            stack_.add(fixed_caption_);

            build_grid(fixed_grid_);
            init_label(fixed_label1_, "1");
            init_label(fixed_label2_, "5");
            init_label(fixed_label3_, "10");
            add_stroke_row(fixed_grid_, fixed_border1_, fixed_label1_, 1, 0);
            add_stroke_row(fixed_grid_, fixed_border2_, fixed_label2_, 5, 1);
            add_stroke_row(fixed_grid_, fixed_border3_, fixed_label3_, 10, 2);
            stack_.add(fixed_grid_);

            // --- Section 2: "Updating the Content Height" (slider-driven content height).
            height_caption_.set_text("Updating the Content Height");
            stack_.add(height_caption_);

            readout_.set_text("Content height: 60");
            stack_.add(readout_);

            // Slider: Minimum 40, Maximum 100; OnAppearing seeds Value = 60.
            height_slider_.set_minimum(40);
            height_slider_.set_maximum(100);
            height_slider_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { apply_content_height(new_value); });
            stack_.add(height_slider_);

            build_grid(height_grid_);
            init_label(height_label1_, "1");
            init_label(height_label2_, "5");
            init_label(height_label3_, "10");
            add_stroke_row(height_grid_, height_border1_, height_label1_, 1, 0);
            add_stroke_row(height_grid_, height_border2_, height_label2_, 5, 1);
            add_stroke_row(height_grid_, height_border3_, height_label3_, 10, 2);
            stack_.add(height_grid_);

            // OnAppearing: ContentHeightSlider.Value = 60 → drives the three tracked labels.
            height_slider_.set_value(60);
            apply_content_height(60);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (each border's content label first, then the border,
        // then its grid, then the stack, then the scroll_view, then the page) so each parent can host its
        // child's native view, then re-host the tree built in the ctor (gallery_attach.hpp). The generic
        // lambda preserves each member's concrete static type — attach_handler keys on the static type.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            // Section 1 leaves → borders → grid.
            gallery_attach_one(app, fixed_caption_, "fixed_caption_");
            gallery_attach_one(app, fixed_label1_, "fixed_label1_");
            gallery_attach_one(app, fixed_border1_, "fixed_border1_");
            gallery_attach_one(app, fixed_label2_, "fixed_label2_");
            gallery_attach_one(app, fixed_border2_, "fixed_border2_");
            gallery_attach_one(app, fixed_label3_, "fixed_label3_");
            gallery_attach_one(app, fixed_border3_, "fixed_border3_");
            gallery_attach_one(app, fixed_grid_, "fixed_grid_");

            // Section 2 leaves → borders → grid (+ caption, readout, slider).
            gallery_attach_one(app, height_caption_, "height_caption_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, height_slider_, "height_slider_");
            gallery_attach_one(app, height_label1_, "height_label1_");
            gallery_attach_one(app, height_border1_, "height_border1_");
            gallery_attach_one(app, height_label2_, "height_label2_");
            gallery_attach_one(app, height_border2_, "height_border2_");
            gallery_attach_one(app, height_label3_, "height_label3_");
            gallery_attach_one(app, height_border3_, "height_border3_");
            gallery_attach_one(app, height_grid_, "height_grid_");

            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroller_, "scroller_");
            gallery_attach_one(app, page_, "page_");

            // Replay the host commands now (the tree was built before any handler existed): each border hosts
            // its label, each grid hosts its borders, the stack hosts both sections, the scroll hosts the
            // stack, the page hosts the scroll.
            gallery_rehost_content(fixed_border1_);
            gallery_rehost_content(fixed_border2_);
            gallery_rehost_content(fixed_border3_);
            gallery_rehost_layout(fixed_grid_);
            gallery_rehost_content(height_border1_);
            gallery_rehost_content(height_border2_);
            gallery_rehost_content(height_border3_);
            gallery_rehost_layout(height_grid_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroller_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::slider& height_slider()
        {
            return height_slider_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::border& fixed_border1()
        {
            return fixed_border1_;
        }
        [[nodiscard]] maui::controls::border& height_border1()
        {
            return height_border1_;
        }

    private:
        // Push the slider value into the three height-tracked content labels + the readout (the C# element
        // HeightRequest bindings + the live-value echo).
        void apply_content_height(double value)
        {
            height_label1_.set_height_request(value);
            height_label2_.set_height_request(value);
            height_label3_.set_height_request(value);

            char text[48];
            std::snprintf(text, sizeof(text), "Content height: %.0f", value);
            readout_.set_text(text);
        }

        // A 3-star-row grid (C# RowDefinitions="*,*,*").
        static void build_grid(maui::controls::grid& grid)
        {
            grid.add_row_definition(maui::core::grid_length::star());
            grid.add_row_definition(maui::core::grid_length::star());
            grid.add_row_definition(maui::core::grid_length::star());
        }

        // One orange, centered content label (C# orange BackgroundColor + centered text alignment).
        static void init_label(maui::controls::label& text, const char* caption)
        {
            text.set_text(caption);
            text.set_horizontal_text_alignment(maui::core::text_alignment::center);
            text.set_vertical_text_alignment(maui::core::text_alignment::center);
            text.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::orange));
        }

        // Place one Rectangle-shape Border (red stroke at `thickness`) wrapping `text` into grid row `row`.
        static void add_stroke_row(maui::controls::grid& grid, maui::controls::border& outline,
                                   maui::controls::label& text, double thickness, int row)
        {
            outline.set_stroke_shape(std::make_shared<maui::graphics::shapes::rectangle>());
            outline.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            outline.set_stroke_thickness(thickness);
            outline.set_content(text);
            grid.add(outline);
            grid.set_row(outline, row);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;

        // Section 1 — fixed-thickness borders over orange labels.
        maui::controls::label fixed_caption_;
        maui::controls::grid fixed_grid_;
        maui::controls::border fixed_border1_;
        maui::controls::label fixed_label1_;
        maui::controls::border fixed_border2_;
        maui::controls::label fixed_label2_;
        maui::controls::border fixed_border3_;
        maui::controls::label fixed_label3_;

        // Section 2 — slider-driven content height.
        maui::controls::label height_caption_;
        maui::controls::label readout_;
        maui::controls::slider height_slider_;
        maui::controls::grid height_grid_;
        maui::controls::border height_border1_;
        maui::controls::label height_label1_;
        maui::controls::border height_border2_;
        maui::controls::label height_label2_;
        maui::controls::border height_border3_;
        maui::controls::label height_label3_;
    };
} // namespace maui::samples
