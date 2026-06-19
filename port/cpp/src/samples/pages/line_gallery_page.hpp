#pragma once
// maui::samples::line_gallery_page — ports LineGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes LineGallery
// (Pages/Controls/ShapesGalleries/LineGallery.xaml + .xaml.cs). The C# code-behind is just
// InitializeComponent() (no shapes built in code-behind), so the whole tree comes from the XAML.
// Unlike the Ellipse/Rectangle galleries there is NO ScrollView here: the StackLayout (Padding 12) is
// the page Content directly. It walks three lines, each under a caption Label —
//   - a basic line: purple stroke, (40,0)->(0,120), in a 120x120 box — "A basic Line";
//   - a dashed line: orange stroke, dashes (1,1) / offset 6, (40,0)->(0,120), 120x120 — "A dash Line";
//   - a thick line: black stroke (thickness 4), (10,10)->(50,50), 50x50 —
//     "A Line using StrokeThickness".
//
// The page OWNS its whole element tree (the shapes_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: this gallery has no ScrollView — the page hosts the StackLayout directly (the C# XAML puts
//         the StackLayout straight under <ContentPage.Content>). The port mirrors that: page_ ->
//         stack_, with no scroll host in between.
//   note: the C# stroke colors are named brushes ("Purple", "Orange", "Black"); the port wraps each in
//         a solid_paint (the shape Stroke accepts a graphics::paint — the documented brush→paint
//         bridge).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class line_gallery_page
    {
    public:
        line_gallery_page()
        {
            page_.set_title("Line Gallery");
            stack_.set_padding(maui::core::thickness{12}); // C# StackLayout Padding="12"

            // --- "A basic Line": purple, (40,0)->(0,120), in a 120x120 box.
            caption(basic_label_, "A basic Line");
            basic_.set_width_request(120);
            basic_.set_height_request(120);
            basic_.set_stroke(solid(maui::graphics::colors::purple));
            basic_.set_x1(40);
            basic_.set_y1(0);
            basic_.set_x2(0);
            basic_.set_y2(120);
            stack_.add(basic_);

            // --- "A dash Line": orange, dashes (1,1) / offset 6, (40,0)->(0,120), 120x120.
            caption(dash_label_, "A dash Line");
            dash_.set_width_request(120);
            dash_.set_height_request(120);
            dash_.set_stroke(solid(maui::graphics::colors::orange));
            dash_.set_stroke_dash_array({1.0, 1.0});
            dash_.set_stroke_dash_offset(6);
            dash_.set_x1(40);
            dash_.set_y1(0);
            dash_.set_x2(0);
            dash_.set_y2(120);
            stack_.add(dash_);

            // --- "A Line using StrokeThickness": black, thickness 4, (10,10)->(50,50), 50x50.
            caption(thick_label_, "A Line using StrokeThickness");
            thick_.set_width_request(50);
            thick_.set_height_request(50);
            thick_.set_x1(10);
            thick_.set_y1(10);
            thick_.set_x2(50);
            thick_.set_y2(50);
            thick_.set_stroke(solid(maui::graphics::colors::black));
            thick_.set_stroke_thickness(4);
            stack_.add(thick_);

            page_.set_content(stack_); // no ScrollView in this gallery — note
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (stack children first in add()-order, then the
        // stack, then the page), then re-host the ctor-built tree (gallery_attach.hpp). The generic
        // gallery_attach_one preserves each member's concrete static type (attach_handler keys on it — an
        // i_view& parameter would erase it to a blank page). No scroll re-host: the page hosts the stack
        // directly.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, basic_label_, "basic_label_");
            gallery_attach_one(app, basic_, "basic_");
            gallery_attach_one(app, dash_label_, "dash_label_");
            gallery_attach_one(app, dash_, "dash_");
            gallery_attach_one(app, thick_label_, "thick_label_");
            gallery_attach_one(app, thick_, "thick_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_); // stack hosts every line + caption
            gallery_rehost_content(page_); // page hosts the stack directly
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named stroke).
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
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label basic_label_;
        maui::controls::shapes::line basic_;
        maui::controls::label dash_label_;
        maui::controls::shapes::line dash_;
        maui::controls::label thick_label_;
        maui::controls::shapes::line thick_;
    };
} // namespace maui::samples
