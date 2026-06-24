#pragma once
// maui::samples::shapes_demo_page — ports ShapesPage.xaml
//
// A self-contained, code-first demo of the MAUI Shapes family. It mirrors the C# gallery page
// (Pages/Controls/ShapesPage.xaml): a ScrollView over a VerticalStackLayout that walks the shape
// catalogue, each shape under a "Headline" caption label —
//   - Ellipse:        custom-color fill (#2B0B98) + red stroke, thickness 4;
//   - Rectangle:      dark-blue fill, red dashed stroke (StrokeDashArray 1,1 / Offset 6);
//   - RoundRectangle: the above plus CornerRadius 12;
//   - Line:           a thick (12) round-capped red horizontal rule;
//   - Polyline:       an open 3-point dark-blue zig-zag with a round line join;
//   - Polygon:        a blue self-intersecting star outline (EvenOdd fill, red stroke);
//   - Path:           a triangle from path markup ("M 10,100 L 100,100 100,50Z"), Uniform aspect; and
//   - a "More samples" Button (the C# OnMoreSamplesClicked navigation entry point).
//
// The page OWNS its whole element tree (the shapes_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: C# RoundRectangle is a Rectangle with a CornerRadius; the port has no separate
//         RoundRectangle shape control, so this row is a rectangle with RadiusX=RadiusY=12 (the exact
//         C# decomposition — Rectangle.GetPath appends a rounded rectangle with max(RadiusX, RadiusY)).
//   note: the custom #2B0B98 Ellipse fill (the C# <Color x:Key="CustomColor">) is reconstructed via
//         color::from_argb — the cross-platform equivalent of the StaticResource brush.
//   note: the C# "More samples" button navigates to the ShapesGalleries sub-gallery
//         (OnMoreSamplesClicked → PushAsync). That sub-gallery navigation is out of scope here, so the
//         button instead drives a readout label (clicked → "More samples tapped") — the same
//         observable click behavior without the deferred navigation host.
//   note: the C# fill/stroke colors are named brushes ("Red", "DarkBlue", "Blue", "Black"); the port
//         wraps each in a solid_paint (the shape Fill/Stroke accept a graphics::paint), the documented
//         brush→paint bridge.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/line_cap.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class shapes_demo_page
    {
    public:
        shapes_demo_page()
        {
            page_.set_title("Shapes");
            stack_.set_spacing(6); // C# VerticalStackLayout Spacing="6"

            // --- Ellipse: custom-color (#2B0B98) fill, red stroke 4, 150x50.
            caption(ellipse_label_, "Ellipse");
            ellipse_.set_fill(solid(maui::graphics::color::from_argb("#2B0B98")));
            ellipse_.set_stroke(solid(maui::graphics::colors::red));
            ellipse_.set_stroke_thickness(4);
            ellipse_.set_width_request(150);
            ellipse_.set_height_request(50);
            stack_.add(ellipse_);

            // --- Rectangle: dark-blue fill, red dashed stroke (dashes 1,1 / offset 6), 150x50.
            caption(rect_label_, "Rectangle");
            rect_.set_fill(solid(maui::graphics::colors::dark_blue));
            rect_.set_stroke(solid(maui::graphics::colors::red));
            rect_.set_stroke_thickness(4);
            rect_.set_stroke_dash_array({1.0, 1.0});
            rect_.set_stroke_dash_offset(6);
            rect_.set_width_request(150);
            rect_.set_height_request(50);
            stack_.add(rect_);

            // --- RoundRectangle: as the rectangle above + CornerRadius 12 (rectangle with radii — note).
            caption(round_rect_label_, "RoundRectangle");
            round_rect_.set_radius_x(12);
            round_rect_.set_radius_y(12);
            round_rect_.set_fill(solid(maui::graphics::colors::dark_blue));
            round_rect_.set_stroke(solid(maui::graphics::colors::red));
            round_rect_.set_stroke_thickness(4);
            round_rect_.set_stroke_dash_array({1.0, 1.0});
            round_rect_.set_stroke_dash_offset(6);
            round_rect_.set_width_request(150);
            round_rect_.set_height_request(50);
            stack_.add(round_rect_);

            // --- Line: (0,20)->(300,20), round caps, red, thickness 12.
            caption(line_label_, "Line");
            line_.set_x1(0);
            line_.set_y1(20);
            line_.set_x2(300);
            line_.set_y2(20);
            line_.set_stroke_line_cap(maui::graphics::line_cap::round);
            line_.set_stroke(solid(maui::graphics::colors::red));
            line_.set_stroke_thickness(12);
            line_.set_width_request(300);
            line_.set_height_request(20);
            stack_.add(line_);

            // --- Polyline: open zig-zag "20 20,250 50,20 120", dark blue, thickness 20, round join.
            caption(polyline_label_, "Polyline");
            polyline_.set_points({{20, 20}, {250, 50}, {20, 120}});
            polyline_.set_stroke(solid(maui::graphics::colors::dark_blue));
            polyline_.set_stroke_thickness(20);
            polyline_.set_stroke_line_join(maui::graphics::line_join::round);
            polyline_.set_width_request(250);
            polyline_.set_height_request(250);
            stack_.add(polyline_);

            // --- Polygon: the C# self-intersecting outline, blue fill (EvenOdd), red stroke 3.
            caption(polygon_label_, "Polygon");
            polygon_.set_points({{0, 48},
                                 {0, 144},
                                 {96, 150},
                                 {100, 0},
                                 {192, 0},
                                 {192, 96},
                                 {50, 96},
                                 {48, 192},
                                 {150, 200},
                                 {144, 48}});
            polygon_.set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
            polygon_.set_fill(solid(maui::graphics::colors::blue));
            polygon_.set_stroke(solid(maui::graphics::colors::red));
            polygon_.set_stroke_thickness(3);
            polygon_.set_width_request(250);
            polygon_.set_height_request(250);
            stack_.add(polygon_);

            // --- Path: a triangle from markup, black stroke, Uniform aspect, 150x150.
            caption(path_label_, "Path");
            auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();
            maui::controls::shapes::parse_path_figure_collection(geometry->figures(), "M 10,100 L 100,100 100,50Z");
            path_.set_data(std::move(geometry));
            path_.set_stroke(solid(maui::graphics::colors::black));
            path_.set_aspect(maui::core::path_aspect::aspect_fit); // C# Aspect="Uniform" → aspect_fit (shape.hpp)
            path_.set_width_request(150);
            path_.set_height_request(150);
            stack_.add(path_);

            // --- "More samples" button (C# OnMoreSamplesClicked; here it drives the readout — note).
            more_samples_.set_text("More samples");
            more_samples_.clicked.connect([this]() { readout_.set_text("More samples tapped"); });
            stack_.add(more_samples_);
            readout_.set_text("Tap 'More samples'");
            stack_.add(readout_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& ellipse_shape()
        {
            return ellipse_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& star()
        {
            return polygon_;
        }
        [[nodiscard]] maui::controls::button& more_samples()
        {
            return more_samples_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named/custom fill or stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // One "Headline"-style caption label above a shape.
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label ellipse_label_;
        maui::controls::shapes::ellipse ellipse_;
        maui::controls::label rect_label_;
        maui::controls::shapes::rectangle rect_;
        maui::controls::label round_rect_label_;
        maui::controls::shapes::rectangle round_rect_;
        maui::controls::label line_label_;
        maui::controls::shapes::line line_;
        maui::controls::label polyline_label_;
        maui::controls::shapes::polyline polyline_;
        maui::controls::label polygon_label_;
        maui::controls::shapes::polygon polygon_;
        maui::controls::label path_label_;
        maui::controls::shapes::path path_;
        maui::controls::button more_samples_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
