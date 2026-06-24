#pragma once
// maui::samples::polygon_gallery_page — ports PolygonGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PolygonGallery.xaml:
// a ScrollView over a StackLayout (Padding 12) that walks four Polygon variants, each under a caption
// Label —
//   - "A basic Polygon":  a 3-point triangle (40,10 70,80 10,50), AliceBlue fill, green stroke 5;
//   - "A dash Polygon":   the same triangle with a dashed stroke (StrokeDashArray 1,1 / Offset 6);
//   - "EvenOdd Polygon":  the C# EvenOddPolygon style (the self-intersecting pentagram point list
//                         10,100 50,0 90,100 0,35 100,35), blue fill, red stroke 3, FillRule EvenOdd;
//   - "NonZero Polygon":  the NonzeroPolygon style (same points), black fill, yellow stroke 3,
//                         FillRule Nonzero.
//
// The EvenOdd vs Nonzero pair is the whole point of the page: identical self-intersecting point lists
// rendered under the two winding rules, so the central pentagram core renders hollow (EvenOdd) vs filled
// (Nonzero). The port reproduces the points / fill / stroke / dash / fill-rule faithfully so the contrast
// renders on macOS + iOS.
//
// The page OWNS its whole element tree (the shapes_page / shapes_demo_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; the page is purely visual.
//   note: the C# <Style x:Key="EvenOddPolygon"/"NonzeroPolygon"> ResourceDictionary styles (Points/Fill/
//         Stroke/StrokeThickness setters) are inlined onto each polygon here — XAML resource styles are a
//         later wave; the resolved per-property values are identical to applying the style.
//   note: the C# fill/stroke colors are named brushes ("AliceBlue", "Green", "Blue", "Red", "Black",
//         "Yellow"); the port wraps each named color in a solid_paint (the documented brush→paint bridge).
//   note: the C# StackLayout (auto-vertical orientation) maps onto vertical_stack_layout; Padding="12"
//         is not modeled on this layout in the port today, so it is omitted (best-effort; the children and
//         their spacing are what the page demonstrates).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class polygon_gallery_page
    {
    public:
        polygon_gallery_page()
        {
            page_.set_title("Polygon Gallery");

            // --- "A basic Polygon": triangle, AliceBlue fill, green stroke 5, 200x100.
            caption(basic_label_, "A basic Polygon");
            basic_.set_points({{40, 10}, {70, 80}, {10, 50}});
            basic_.set_fill(solid(maui::graphics::colors::alice_blue));
            basic_.set_stroke(solid(maui::graphics::colors::green));
            basic_.set_stroke_thickness(5);
            basic_.set_width_request(200);
            basic_.set_height_request(100);
            stack_.add(basic_);

            // --- "A dash Polygon": same triangle, green dashed stroke (dashes 1,1 / offset 6), 200x100.
            caption(dash_label_, "A dash Polygon");
            dash_.set_points({{40, 10}, {70, 80}, {10, 50}});
            dash_.set_fill(solid(maui::graphics::colors::alice_blue));
            dash_.set_stroke(solid(maui::graphics::colors::green));
            dash_.set_stroke_thickness(5);
            dash_.set_stroke_dash_array({1.0, 1.0});
            dash_.set_stroke_dash_offset(6);
            dash_.set_width_request(200);
            dash_.set_height_request(100);
            stack_.add(dash_);

            // --- "EvenOdd Polygon": C# EvenOddPolygon style — pentagram point list, blue fill, red stroke 3,
            //     FillRule EvenOdd (the self-intersecting core renders hollow), 100x100.
            caption(even_odd_label_, "EvenOdd Polygon");
            even_odd_.set_points({{10, 100}, {50, 0}, {90, 100}, {0, 35}, {100, 35}});
            even_odd_.set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
            even_odd_.set_fill(solid(maui::graphics::colors::blue));
            even_odd_.set_stroke(solid(maui::graphics::colors::red));
            even_odd_.set_stroke_thickness(3);
            even_odd_.set_width_request(100);
            even_odd_.set_height_request(100);
            stack_.add(even_odd_);

            // --- "NonZero Polygon": C# NonzeroPolygon style — same point list, black fill, yellow stroke 3,
            //     FillRule Nonzero (the self-intersecting core renders filled), 100x100.
            caption(nonzero_label_, "NonZero Polygon");
            nonzero_.set_points({{10, 100}, {50, 0}, {90, 100}, {0, 35}, {100, 35}});
            nonzero_.set_fill_rule(maui::controls::shapes::fill_rule::nonzero);
            nonzero_.set_fill(solid(maui::graphics::colors::black));
            nonzero_.set_stroke(solid(maui::graphics::colors::yellow));
            nonzero_.set_stroke_thickness(3);
            nonzero_.set_width_request(100);
            nonzero_.set_height_request(100);
            stack_.add(nonzero_);

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
        [[nodiscard]] maui::controls::shapes::polygon& basic()
        {
            return basic_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& even_odd()
        {
            return even_odd_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& nonzero()
        {
            return nonzero_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named fill or stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // One caption label above a polygon.
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label basic_label_;
        maui::controls::shapes::polygon basic_;
        maui::controls::label dash_label_;
        maui::controls::shapes::polygon dash_;
        maui::controls::label even_odd_label_;
        maui::controls::shapes::polygon even_odd_;
        maui::controls::label nonzero_label_;
        maui::controls::shapes::polygon nonzero_;
    };
} // namespace maui::samples
