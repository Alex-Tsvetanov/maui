#pragma once
// maui::samples::polyline_gallery_page — ports PolylineGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolylineGallery.xaml:
// a StackLayout (Padding 12 — no ScrollView in the C# source) holding two Polyline variants, each under a
// caption Label —
//   - "A basic Polyline": an open 10-point zig-zag (0,0 10,30 15,0 18,60 23,30 35,30 40,0 43,60 48,30
//                         100,30), red stroke (default thickness), 500x100;
//   - "A dash Polyline":  the same 10-point zig-zag with a red dashed stroke (StrokeThickness 2,
//                         StrokeDashArray 1,1 / StrokeDashOffset 6), 500x100.
//
// Polyline (unlike Polygon) does NOT auto-close, so the demo is an open connected run of line segments —
// the port reproduces the exact point list, stroke color, thickness and dash pattern so it renders on
// macOS + iOS.
//
// The page OWNS its whole element tree (the shapes_page / shapes_demo_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; the page is purely visual.
//   note: the C# page has NO ScrollView (just a StackLayout) — faithfully mirrored; the page owns a
//         vertical_stack_layout directly as the page content.
//   note: the basic Polyline sets no StrokeThickness in XAML, so it uses the Shape default (1) — left
//         unset here to match (the dash Polyline sets StrokeThickness 2 explicitly).
//   note: the stroke color is the named brush "Red"; the port wraps the named color in a solid_paint
//         (the documented brush→paint bridge).
//   note: the C# StackLayout Padding="12" is not modeled on this layout in the port today, so it is
//         omitted (best-effort; the two polylines and their order are what the page demonstrates).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class polyline_gallery_page
    {
    public:
        polyline_gallery_page()
        {
            page_.set_title("Polyline Gallery");

            // --- "A basic Polyline": open 10-point zig-zag, red stroke (default thickness), 500x100.
            caption(basic_label_, "A basic Polyline");
            basic_.set_points(
                {{0, 0}, {10, 30}, {15, 0}, {18, 60}, {23, 30}, {35, 30}, {40, 0}, {43, 60}, {48, 30}, {100, 30}});
            basic_.set_stroke(solid(maui::graphics::colors::red));
            basic_.set_width_request(500);
            basic_.set_height_request(100);
            stack_.add(basic_);

            // --- "A dash Polyline": same zig-zag, red dashed stroke (thickness 2, dashes 1,1 / offset 6),
            //     500x100.
            caption(dash_label_, "A dash Polyline");
            dash_.set_points(
                {{0, 0}, {10, 30}, {15, 0}, {18, 60}, {23, 30}, {35, 30}, {40, 0}, {43, 60}, {48, 30}, {100, 30}});
            dash_.set_stroke(solid(maui::graphics::colors::red));
            dash_.set_stroke_thickness(2);
            dash_.set_stroke_dash_array({1.0, 1.0});
            dash_.set_stroke_dash_offset(6);
            dash_.set_width_request(500);
            dash_.set_height_request(100);
            stack_.add(dash_);

            page_.set_content(stack_);
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
        [[nodiscard]] maui::controls::shapes::polyline& basic()
        {
            return basic_;
        }
        [[nodiscard]] maui::controls::shapes::polyline& dash()
        {
            return dash_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // One caption label above a polyline.
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label basic_label_;
        maui::controls::shapes::polyline basic_;
        maui::controls::label dash_label_;
        maui::controls::shapes::polyline dash_;
    };
} // namespace maui::samples
