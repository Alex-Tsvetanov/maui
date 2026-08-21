#pragma once
// maui::samples::border_resize_content_page — ports BorderResizeContent.xaml
//
// A self-contained, code-first demo that resizes a Border's CONTENT and watches the Border track it. It
// mirrors the C# core gallery page (Pages/Core/BorderGalleries/BorderResizeContent.xaml): a centered
// VerticalStackLayout over a 2-column x 3-row Grid of six Borders. Each Border is 101x101, LightBlue
// background, an 8-unit LightGreen stroke, and one of three StrokeShapes — an Ellipse (row 0), a
// RoundRectangle (row 1), and a Polygon triangle (row 2). Column 0 holds a Label whose Text + FontSize
// are driven by the controls below; column 1 holds an Image (oasis.jpg) whose Scale is driven by a
// slider. As the content grows the Border re-measures around it (Border.CrossPlatformMeasure insets the
// content by Padding + StrokeThickness), which is the behavior the page exists to show.
//
// The three C# Styles (BorderStyleCircle / RoundRectangle / Triangle) carry identical box properties
// (101x101, LightBlue, stroke 8, LightGreen) and differ only in StrokeShape; the port applies those same
// property values directly in the ctor (the headless-safe equivalent of the keyed-Style application —
// the style layer would resolve value-for-value the same). The C# ButtonIconStyle Label (64-pt, centered,
// #99FF0000 wash, #0088ee text) is reproduced on each label.
//
// XAML bindings, reproduced as live event wiring (the port has no XAML binding engine on this layer):
//   - Label.Text   = {Binding Text,  Source=TextEntry}        -> entry text_changed -> set each label text
//   - Label.FontSize= {Binding Value, Source=FontSizeSlider}  -> slider value_changed -> set each label font size
//   - Image.Scale  = {Binding Value, Source=ImageScaleSlider} -> slider value_changed -> set each image scale
// The initial values match the XAML (Entry "+", FontSize 40, ImageScale 1).
//
// note: oasis.jpg is the C# sample's bundled asset; the port references it by file name through
//       image_source::from_file (the file resolves on a device build that bundles the asset; the headless
//       backend simply carries the source). The Image.Scale binding is wired and live; the visible scale
//       depends on the native image being present.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <array>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/path_segment.hpp" // point_collection
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class border_resize_content_page
    {
    public:
        border_resize_content_page()
        {
            page_.set_title("Border resize Content");
            stack_.set_spacing(10);
            stack_.set_padding(
                maui::core::thickness(16)); // shared XAML: <VerticalStackLayout Spacing="10" Padding="16">

            // Grid: ColumnDefinitions "*,*", RowDefinitions "*,*,*", 10-unit spacing both axes.
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.set_column_spacing(10);
            grid_.set_row_spacing(10);

            // The three shape-bearing border styles down each column (row 0 ellipse, 1 round-rect, 2 triangle).
            style_border(circle_label_border_, std::make_shared<maui::graphics::shapes::ellipse>());
            style_border(round_label_border_, std::make_shared<maui::graphics::shapes::round_rectangle>(8.0));
            style_border(triangle_label_border_, make_triangle());
            style_border(circle_image_border_, std::make_shared<maui::graphics::shapes::ellipse>());
            style_border(round_image_border_, std::make_shared<maui::graphics::shapes::round_rectangle>(8.0));
            style_border(triangle_image_border_, make_triangle());

            // Column 0 — the three labels (the Content whose Text + FontSize the controls drive).
            for (auto* lbl : labels())
            {
                style_icon_label(*lbl);
            }
            circle_label_border_.set_content(circle_label_);
            round_label_border_.set_content(round_label_);
            triangle_label_border_.set_content(triangle_label_);

            // Column 1 — the three images (Scale driven by the image-scale slider).
            for (auto* img : images())
            {
                img->set_source(maui::controls::image_source::from_file("oasis.jpg"));
            }
            circle_image_border_.set_content(circle_image_);
            round_image_border_.set_content(round_image_);
            triangle_image_border_.set_content(triangle_image_);

            // Place the six borders into the grid (column 0 = labels, column 1 = images).
            place(circle_label_border_, 0, 0);
            place(round_label_border_, 1, 0);
            place(triangle_label_border_, 2, 0);
            place(circle_image_border_, 0, 1);
            place(round_image_border_, 1, 1);
            place(triangle_image_border_, 2, 1);

            // The controls below the grid: Entry "+", FontSize slider [20..200]=40, Scale slider [1..20]=1.
            content_text_header_.set_text("Content Text");
            text_entry_.set_text("+");
            font_size_header_.set_text("Content Text FontSize");
            font_size_slider_.set_minimum(20);
            font_size_slider_.set_maximum(200);
            font_size_slider_.set_value(40);
            image_scale_header_.set_text("Image Scale");
            image_scale_slider_.set_minimum(1);
            image_scale_slider_.set_maximum(20);
            image_scale_slider_.set_value(1);

            // Wire the three XAML bindings as live event connections.
            text_entry_.text_changed.connect(
                [this](const std::string& /*old_text*/, const std::string& new_text) { apply_label_text(new_text); });
            font_size_slider_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { apply_label_font_size(new_value); });
            image_scale_slider_.value_changed.connect(
                [this](double /*old_value*/, double new_value) { apply_image_scale(new_value); });

            // Seed the initial bound values (the XAML initial state).
            //
            // The FontSize slider is NOT seeded. GROUND-TRUTH-ROOT makes port/maui-reference/pages/
            // border_resize_content.xaml the page of record, and there the six content Labels carry
            // FontSize="64" while the "Content Text FontSize" Slider (Value="40") drives nothing at rest —
            // it has no binding and the twin has no code-behind. Seeding 40 here overwrote make_label's 64
            // and rendered every "+" at 57x58px against the reference's 92x95 (MEASURED on
            // border_resize_content_light @3x; the xaml column, which loads the twin, draws 92x95 — the
            // FOUR-COMPARISONS column split is what localised this to the code-first builder). The slider
            // still drives the size on CHANGE, which is the interactivity the C# sample's binding provides.
            apply_label_text("+");
            apply_image_scale(1);

            stack_.add(grid_);
            stack_.add(content_text_header_);
            stack_.add(text_entry_);
            stack_.add(font_size_header_);
            stack_.add(font_size_slider_);
            stack_.add(image_scale_header_);
            stack_.add(image_scale_slider_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::entry& text_entry()
        {
            return text_entry_;
        }
        [[nodiscard]] maui::controls::slider& font_size_slider()
        {
            return font_size_slider_;
        }
        [[nodiscard]] maui::controls::slider& image_scale_slider()
        {
            return image_scale_slider_;
        }

    private:
        std::array<maui::controls::label*, 3> labels()
        {
            return {&circle_label_, &round_label_, &triangle_label_};
        }
        std::array<maui::controls::image*, 3> images()
        {
            return {&circle_image_, &round_image_, &triangle_image_};
        }

        // The shared box properties of all three border styles (101x101, LightBlue, stroke 8, LightGreen),
        // differing only by the StrokeShape passed in.
        static void style_border(maui::controls::border& border, std::shared_ptr<maui::graphics::i_shape> shape)
        {
            border.set_width_request(101);
            border.set_height_request(101);
            border.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_blue));
            border.set_stroke_thickness(8);
            border.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_green));
            border.set_stroke_shape(std::move(shape));
        }

        // ButtonIconStyle: a 64-pt centered label with a #99FF0000 background wash + #0088ee text.
        static void style_icon_label(maui::controls::label& label)
        {
            label.set_font(maui::core::font::system_font_of_size(64));
            label.set_horizontal_text_alignment(maui::core::text_alignment::center);
            label.set_vertical_text_alignment(maui::core::text_alignment::center);
            label.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::color::from_rgba(255, 0, 0, 0x99)));
            label.set_text_color(maui::graphics::color::from_argb("#0088ee"));
        }

        // The C# Polygon triangle (Points="40,10 70,80 10,50"), used as a clip StrokeShape.
        static std::shared_ptr<maui::controls::shapes::polygon> make_triangle()
        {
            maui::controls::shapes::point_collection points{
                maui::graphics::point{40, 10}, maui::graphics::point{70, 80}, maui::graphics::point{10, 50}};
            return std::make_shared<maui::controls::shapes::polygon>(std::move(points));
        }

        void place(maui::controls::border& border, int row, int column)
        {
            grid_.add(border);
            grid_.set_row(border, row);
            grid_.set_column(border, column);
        }

        // The three XAML bindings, applied across the three label / image instances.
        void apply_label_text(const std::string& text)
        {
            for (auto* lbl : labels())
            {
                lbl->set_text(text);
            }
        }
        void apply_label_font_size(double size)
        {
            for (auto* lbl : labels())
            {
                lbl->set_font(lbl->font().with_size(size));
            }
        }
        void apply_image_scale(double scale)
        {
            for (auto* img : images())
            {
                img->set_scale(scale);
            }
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::grid grid_;

        // Column 0 — the three label borders + their labels.
        maui::controls::border circle_label_border_;
        maui::controls::border round_label_border_;
        maui::controls::border triangle_label_border_;
        maui::controls::label circle_label_;
        maui::controls::label round_label_;
        maui::controls::label triangle_label_;

        // Column 1 — the three image borders + their images.
        maui::controls::border circle_image_border_;
        maui::controls::border round_image_border_;
        maui::controls::border triangle_image_border_;
        maui::controls::image circle_image_;
        maui::controls::image round_image_;
        maui::controls::image triangle_image_;

        // The controls below the grid.
        maui::controls::label content_text_header_;
        maui::controls::entry text_entry_;
        maui::controls::label font_size_header_;
        maui::controls::slider font_size_slider_;
        maui::controls::label image_scale_header_;
        maui::controls::slider image_scale_slider_;
    };
} // namespace maui::samples
