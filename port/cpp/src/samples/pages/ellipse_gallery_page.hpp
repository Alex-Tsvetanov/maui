#pragma once
// maui::samples::ellipse_gallery_page — ports EllipseGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes EllipseGallery
// (Pages/Controls/ShapesGalleries/EllipseGallery.xaml + .xaml.cs). The C# code-behind is just
// InitializeComponent() (no shapes built in code-behind), so the whole tree comes from the XAML:
// a ScrollView over a StackLayout (Padding 12) walking a catalogue of ellipses, each under a
// caption Label —
//   - ellipse (x:Name): red FILL, 150x50 — "A basic Rectangle" (the C# caption text, verbatim);
//   - a circle: red stroke (thickness 4), no fill, 150x150 — "A Circle";
//   - an ellipse with stroke: red stroke 4, no fill, 150x50 — "An Ellipse with stroke";
//   - the same plus a dark-blue fill, 150x50 (shares the caption above);
//   - a dashed ellipse: dark-blue fill, red stroke 4, dashes (1,1) / offset 6, 150x50 —
//     "An Ellipse with stroke dash".
//
// The page OWNS its whole element tree (the shapes_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# <Style TargetType="Ellipse"> sets HorizontalOptions="Start" on every ellipse. The
//         port's public view surface has no per-view HorizontalOptions setter (horizontal_layout_-
//         alignment is fixed to Fill on view<>), so the Start alignment is not reproduced — each shape
//         still pins its own WidthRequest/HeightRequest, so the visual ellipse sizes are faithful; only
//         the in-stack horizontal alignment differs.
//   note: the first caption text really is "A basic Rectangle" in the C# XAML (a copy/paste artifact
//         from RectangleGallery) — preserved verbatim for fidelity rather than corrected.
//   note: the C# fill/stroke colors are named brushes ("Red", "DarkBlue"); the port wraps each in a
//         solid_paint (the shape Fill/Stroke accept a graphics::paint — the documented brush→paint
//         bridge).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class ellipse_gallery_page
    {
    public:
        ellipse_gallery_page()
        {
            page_.set_title("Ellipse Gallery");
            stack_.set_padding(maui::core::thickness{12}); // C# StackLayout Padding="12"

            // --- "A basic Rectangle" (verbatim C# caption — note) + the x:Name="ellipse": red fill, 150x50.
            caption(basic_label_, "A basic Rectangle");
            ellipse_.set_fill(solid(maui::graphics::colors::red));
            ellipse_.set_width_request(150);
            ellipse_.set_height_request(50);
            stack_.add(ellipse_);

            // --- "A Circle": red stroke 4, no fill, 150x150.
            caption(circle_label_, "A Circle");
            circle_.set_stroke(solid(maui::graphics::colors::red));
            circle_.set_stroke_thickness(4);
            circle_.set_width_request(150);
            circle_.set_height_request(150);
            stack_.add(circle_);

            // --- "An Ellipse with stroke": red stroke 4, no fill, 150x50.
            caption(stroke_label_, "An Ellipse with stroke");
            stroke_ellipse_.set_stroke(solid(maui::graphics::colors::red));
            stroke_ellipse_.set_stroke_thickness(4);
            stroke_ellipse_.set_width_request(150);
            stroke_ellipse_.set_height_request(50);
            stack_.add(stroke_ellipse_);

            // --- the same with a dark-blue fill (shares the caption above), 150x50.
            filled_stroke_ellipse_.set_fill(solid(maui::graphics::colors::dark_blue));
            filled_stroke_ellipse_.set_stroke(solid(maui::graphics::colors::red));
            filled_stroke_ellipse_.set_stroke_thickness(4);
            filled_stroke_ellipse_.set_width_request(150);
            filled_stroke_ellipse_.set_height_request(50);
            stack_.add(filled_stroke_ellipse_);

            // --- "An Ellipse with stroke dash": dark-blue fill, red dashed stroke (1,1 / offset 6), 150x50.
            caption(dash_label_, "An Ellipse with stroke dash");
            dash_ellipse_.set_fill(solid(maui::graphics::colors::dark_blue));
            dash_ellipse_.set_stroke(solid(maui::graphics::colors::red));
            dash_ellipse_.set_stroke_thickness(4);
            dash_ellipse_.set_stroke_dash_array({1.0, 1.0});
            dash_ellipse_.set_stroke_dash_offset(6);
            dash_ellipse_.set_width_request(150);
            dash_ellipse_.set_height_request(50);
            stack_.add(dash_ellipse_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (stack children first in add()-order, then the
        // stack, then the scroll_view, then the page), then re-host the ctor-built tree (gallery_attach.hpp).
        // The generic gallery_attach_one preserves each member's concrete static type (attach_handler keys
        // on it — an i_view& parameter would erase it to a blank page).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, basic_label_, "basic_label_");
            gallery_attach_one(app, ellipse_, "ellipse_");
            gallery_attach_one(app, circle_label_, "circle_label_");
            gallery_attach_one(app, circle_, "circle_");
            gallery_attach_one(app, stroke_label_, "stroke_label_");
            gallery_attach_one(app, stroke_ellipse_, "stroke_ellipse_");
            gallery_attach_one(app, filled_stroke_ellipse_, "filled_stroke_ellipse_");
            gallery_attach_one(app, dash_label_, "dash_label_");
            gallery_attach_one(app, dash_ellipse_, "dash_ellipse_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroll_, "scroll_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);   // stack hosts every ellipse + caption
            gallery_rehost_content(scroll_); // scroll hosts the stack
            gallery_rehost_content(page_);   // page hosts the scroll
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
        maui::controls::shapes::ellipse ellipse_; // C# x:Name="ellipse"
        maui::controls::label circle_label_;
        maui::controls::shapes::ellipse circle_;
        maui::controls::label stroke_label_;
        maui::controls::shapes::ellipse stroke_ellipse_;
        maui::controls::shapes::ellipse filled_stroke_ellipse_;
        maui::controls::label dash_label_;
        maui::controls::shapes::ellipse dash_ellipse_;
    };
} // namespace maui::samples
