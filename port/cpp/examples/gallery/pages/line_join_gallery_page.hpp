#pragma once
// maui::samples::line_join_gallery_page — ports LineJoinGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/LineJoinGallery.xaml: a StackLayout that demonstrates the three
// StrokeLineJoin variants on an identical open polyline. The C# page defines a shared "PolylineStyle"
// resource (Points "20 20, 250 50, 20 120", Stroke Aqua, StrokeThickness 20) and applies it to three
// Polylines, each wrapped in a Grid and captioned, differing only by StrokeLineJoin —
//   - Miter (default): sharp corner;
//   - Bevel:           beveled corner (triangle across the outer edges);
//   - Round:           rounded corner.
//
// The thick (20px) stroke and the sharp ~中-angle at the middle vertex make the three joins visually
// distinct, which is the whole point of the gallery.
//
// The page OWNS its whole element tree (the shapes_demo_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# <Style x:Key="PolylineStyle"> StaticResource is not a XAML resource here (XAML is
//         layer 6, deferred). Each polyline is configured directly with the style's setter values
//         (Points / Stroke=Aqua / StrokeThickness=20) — the exact resolved style, code-first.
//   note: each C# Polyline sits inside its own <Grid>. The port reproduces that single-child Grid
//         wrapper so the layout/host nesting matches; the Grid has one implied star row/column, so the
//         polyline fills it as in C#.
//   note: Stroke="Aqua" → solid_paint over colors::aqua (the C# named-brush → paint bridge).

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/line_join.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class line_join_gallery_page
    {
    public:
        line_join_gallery_page()
        {
            page_.set_title("LineJoin Gallery");

            // --- Miter (default).
            miter_label_.set_text("Miter (default):");
            stack_.add(miter_label_);
            style_polyline(miter_, maui::graphics::line_join::miter);
            miter_grid_.add(miter_);
            stack_.add(miter_grid_);

            // --- Bevel.
            bevel_label_.set_text("Bevel:");
            stack_.add(bevel_label_);
            style_polyline(bevel_, maui::graphics::line_join::bevel);
            bevel_grid_.add(bevel_);
            stack_.add(bevel_grid_);

            // --- Round.
            round_label_.set_text("Round:");
            stack_.add(round_label_);
            style_polyline(round_, maui::graphics::line_join::round);
            round_grid_.add(round_);
            stack_.add(round_grid_);

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::shapes::polyline& miter()
        {
            return miter_;
        }
        [[nodiscard]] maui::controls::shapes::polyline& bevel()
        {
            return bevel_;
        }
        [[nodiscard]] maui::controls::shapes::polyline& round()
        {
            return round_;
        }

    private:
        // Apply the shared C# PolylineStyle (Points / Aqua stroke / thickness 20) + the per-instance join.
        static void style_polyline(maui::controls::shapes::polyline& line, maui::graphics::line_join join)
        {
            line.set_points({{20, 20}, {250, 50}, {20, 120}});
            line.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::aqua));
            line.set_stroke_thickness(20);
            line.set_stroke_line_join(join);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label miter_label_;
        maui::controls::grid miter_grid_;
        maui::controls::shapes::polyline miter_;
        maui::controls::label bevel_label_;
        maui::controls::grid bevel_grid_;
        maui::controls::shapes::polyline bevel_;
        maui::controls::label round_label_;
        maui::controls::grid round_grid_;
        maui::controls::shapes::polyline round_;
    };
} // namespace maui::samples
