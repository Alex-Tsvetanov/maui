#pragma once
// shapes_page — a faithful reproduction of the maui-compare "shapes" demo (ComparePages.Shapes()), the
// shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView over a vertical stack of four
// LABELLED shapes, each bold-captioned and Start-aligned at a fixed request size:
//   - Ellipse  (Fill Red, Stroke DarkBlue, StrokeThickness 4, 150x60),
//   - RoundRectangle  (a Rectangle, Fill DarkBlue, RadiusX 12, RadiusY 24, 150x60),
//   - EvenOdd Polygon (pentagram)  (Points {(10,100),(50,0),(90,100),(0,35),(100,35)}, Fill Blue, Stroke
//     Red, StrokeThickness 2, FillRule EvenOdd, 100x100),
//   - Line  (X1 40 Y1 0 -> X2 0 Y2 80, Stroke Purple, StrokeThickness 2, 120x80).
// Kept 1:1 with the C# reference (same shapes, fills, strokes, sizes, captions, order) so the side-by-side
// gallery comparison is apples-to-apples.
//
// The page OWNS its whole element tree (the containers_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/
// ios test trees exercise the same wiring directly.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class shapes_page
    {
        static constexpr maui::core::layout_alignment k_start = maui::core::layout_alignment::start;

        static std::shared_ptr<maui::graphics::solid_paint> paint(const maui::graphics::color& color)
        {
            return std::make_shared<maui::graphics::solid_paint>(color);
        }

    public:
        shapes_page()
        {
            page_.set_title("Shapes");
            stack_.set_spacing(16);
            stack_.set_padding(maui::core::thickness(16)); // ComparePages: Padding=16, Spacing=16.

            // Each caption is a bold Label (FontAttributes.Bold), at the default system size.
            const auto bold = maui::core::font::system_font_of_weight(maui::core::font_weight::bold);

            // --- Ellipse: Fill Red, Stroke DarkBlue (4), 150x60, Start. ---
            ellipse_label_.set_text("Ellipse");
            ellipse_label_.set_font(bold);
            ellipse_.set_fill(paint(maui::graphics::colors::red));
            ellipse_.set_stroke(paint(maui::graphics::colors::dark_blue));
            ellipse_.set_stroke_thickness(4);
            ellipse_.set_width_request(150);
            ellipse_.set_height_request(60);
            ellipse_.set_horizontal_layout_alignment(k_start);

            // --- RoundRectangle: a Rectangle, Fill DarkBlue, RadiusX 12 / RadiusY 24, 150x60, Start. ---
            rect_label_.set_text("RoundRectangle");
            rect_label_.set_font(bold);
            rect_.set_fill(paint(maui::graphics::colors::dark_blue));
            rect_.set_radius_x(12);
            rect_.set_radius_y(24);
            rect_.set_width_request(150);
            rect_.set_height_request(60);
            rect_.set_horizontal_layout_alignment(k_start);

            // --- EvenOdd Polygon (pentagram): Fill Blue, Stroke Red (2), EvenOdd, 100x100, Start. ---
            poly_label_.set_text("EvenOdd Polygon (pentagram)");
            poly_label_.set_font(bold);
            poly_.set_points({{10, 100}, {50, 0}, {90, 100}, {0, 35}, {100, 35}});
            poly_.set_fill(paint(maui::graphics::colors::blue));
            poly_.set_stroke(paint(maui::graphics::colors::red));
            poly_.set_stroke_thickness(2);
            poly_.set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
            poly_.set_width_request(100);
            poly_.set_height_request(100);
            poly_.set_horizontal_layout_alignment(k_start);

            // --- Line: (40,0)->(0,80), Stroke Purple (2), 120x80, Start. ---
            line_label_.set_text("Line");
            line_label_.set_font(bold);
            line_.set_x1(40);
            line_.set_y1(0);
            line_.set_x2(0);
            line_.set_y2(80);
            line_.set_stroke(paint(maui::graphics::colors::purple));
            line_.set_stroke_thickness(2);
            line_.set_width_request(120);
            line_.set_height_request(80);
            line_.set_horizontal_layout_alignment(k_start);

            stack_.add(ellipse_label_);
            stack_.add(ellipse_);
            stack_.add(rect_label_);
            stack_.add(rect_);
            stack_.add(poly_label_);
            stack_.add(poly_);
            stack_.add(line_label_);
            stack_.add(line_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& ellipse()
        {
            return ellipse_;
        }
        [[nodiscard]] maui::controls::shapes::rectangle& rect()
        {
            return rect_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& polygon()
        {
            return poly_;
        }
        [[nodiscard]] maui::controls::shapes::line& line()
        {
            return line_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label ellipse_label_;
        maui::controls::shapes::ellipse ellipse_;
        maui::controls::label rect_label_;
        maui::controls::shapes::rectangle rect_;
        maui::controls::label poly_label_;
        maui::controls::shapes::polygon poly_;
        maui::controls::label line_label_;
        maui::controls::shapes::line line_;
    };
} // namespace maui::samples
