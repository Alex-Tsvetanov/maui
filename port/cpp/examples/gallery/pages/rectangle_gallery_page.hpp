#pragma once
// maui::samples::rectangle_gallery_page — ports RectangleGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes RectangleGallery
// (Pages/Controls/ShapesGalleries/RectangleGallery.xaml + .xaml.cs). The C# code-behind is just
// InitializeComponent() (no shapes built in code-behind), so the whole tree comes from the XAML:
// a ScrollView over a StackLayout (Padding 12) walking a catalogue of rectangles, each under a
// caption Label —
//   - a basic rectangle: red FILL, 150x50 — "A basic Rectangle";
//   - a square: red stroke (thickness 4), no fill, 150x150 — "A Square";
//   - a rectangle with stroke: red stroke 4, no fill, 150x50 — "A Rectangle with stroke";
//   - the same plus a dark-blue fill, 150x50 (shares the caption above);
//   - a dashed rectangle: dark-blue fill, red stroke 4, dashes (1,1) / offset 6, 150x50 —
//     "A Rectangle with stroke dash";
//   - a rounded rectangle: RadiusX 12 / RadiusY 24, dark-blue fill, 150x50 —
//     "A Rectangle with curved corners".
//
// The page OWNS its whole element tree (the shapes_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the maui-compare RectangleGalleryPage.cs (the app rendered in the MAUI reference column) does
//         NOT set HorizontalOptions on the rectangles — so each rectangle's default Fill alignment plus an
//         explicit WidthRequest is treated as Center by MAUI's LayoutExtensions.AlignHorizontal (the
//         Fill+explicit-width → Center rule the port mirrors in view::align_horizontal). The rectangles
//         therefore center, matching the MAUI capture; the port sets no Start override.
//   note: the C# curved-corners rectangle has RadiusX="12" RadiusY="24" — but the port's Rectangle
//         (faithful to Rectangle.cs GetPath, which keeps the C# `TODO: one radius`) draws corners with
//         max(RadiusX, RadiusY) = 24. Both radii are still set so the property surface matches; the
//         single-radius rounding is the documented C# behavior, not a port shortcut.
//   note: the C# fill/stroke colors are named brushes ("Red", "DarkBlue"); the port wraps each in a
//         solid_paint (the shape Fill/Stroke accept a graphics::paint — the documented brush→paint
//         bridge).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class rectangle_gallery_page
    {
    public:
        rectangle_gallery_page()
        {
            page_.set_title("Rectangle Gallery");
            stack_.set_padding(maui::core::thickness{12}); // C# StackLayout Padding="12"

            // --- "A basic Rectangle": red fill, 150x50.
            caption(basic_label_, "A basic Rectangle");
            basic_.set_fill(solid(maui::graphics::colors::red));
            basic_.set_width_request(150);
            basic_.set_height_request(50);
            stack_.add(basic_);

            // --- "A Square": red stroke 4, no fill, 150x150.
            caption(square_label_, "A Square");
            square_.set_stroke(solid(maui::graphics::colors::red));
            square_.set_stroke_thickness(4);
            square_.set_width_request(150);
            square_.set_height_request(150);
            stack_.add(square_);

            // --- "A Rectangle with stroke": red stroke 4, no fill, 150x50.
            caption(stroke_label_, "A Rectangle with stroke");
            stroke_rect_.set_stroke(solid(maui::graphics::colors::red));
            stroke_rect_.set_stroke_thickness(4);
            stroke_rect_.set_width_request(150);
            stroke_rect_.set_height_request(50);
            stack_.add(stroke_rect_);

            // --- the same with a dark-blue fill (shares the caption above), 150x50.
            filled_stroke_rect_.set_fill(solid(maui::graphics::colors::dark_blue));
            filled_stroke_rect_.set_stroke(solid(maui::graphics::colors::red));
            filled_stroke_rect_.set_stroke_thickness(4);
            filled_stroke_rect_.set_width_request(150);
            filled_stroke_rect_.set_height_request(50);
            stack_.add(filled_stroke_rect_);

            // --- "A Rectangle with stroke dash": dark-blue fill, red dashed stroke (1,1 / offset 6), 150x50.
            caption(dash_label_, "A Rectangle with stroke dash");
            dash_rect_.set_fill(solid(maui::graphics::colors::dark_blue));
            dash_rect_.set_stroke(solid(maui::graphics::colors::red));
            dash_rect_.set_stroke_thickness(4);
            dash_rect_.set_stroke_dash_array({1.0, 1.0});
            dash_rect_.set_stroke_dash_offset(6);
            dash_rect_.set_width_request(150);
            dash_rect_.set_height_request(50);
            stack_.add(dash_rect_);

            // --- "A Rectangle with curved corners": RadiusX 12 / RadiusY 24 (port rounds with max — note),
            //     dark-blue fill, 150x50.
            caption(curved_label_, "A Rectangle with curved corners");
            curved_rect_.set_radius_x(12);
            curved_rect_.set_radius_y(24);
            curved_rect_.set_fill(solid(maui::graphics::colors::dark_blue));
            curved_rect_.set_width_request(150);
            curved_rect_.set_height_request(50);
            stack_.add(curved_rect_);

            // C# RectangleGalleryPage (maui-compare, the rendered ground truth) does NOT set
            // HorizontalOptions on the rectangles, so they keep the default Fill+explicit-width behaviour
            // which view::align_horizontal centers — matching the MAUI capture. (No Start override here.)

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named fill or stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // One caption label above a shape (the C# <Label Text="..."/> rows).
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label basic_label_;
        maui::controls::shapes::rectangle basic_;
        maui::controls::label square_label_;
        maui::controls::shapes::rectangle square_;
        maui::controls::label stroke_label_;
        maui::controls::shapes::rectangle stroke_rect_;
        maui::controls::shapes::rectangle filled_stroke_rect_;
        maui::controls::label dash_label_;
        maui::controls::shapes::rectangle dash_rect_;
        maui::controls::label curved_label_;
        maui::controls::shapes::rectangle curved_rect_;
    };
} // namespace maui::samples
