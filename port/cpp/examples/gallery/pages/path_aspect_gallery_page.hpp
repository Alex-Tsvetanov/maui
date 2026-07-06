#pragma once
// maui::samples::path_aspect_gallery_page — ports PathAspectGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/PathAspectGallery.xaml: a StackLayout (Padding 12) that demonstrates
// the four Path Aspect modes on one identical geometry. Each Path is 100x100, BackgroundColor LightGray,
// Stroke Yellow, Fill Red, StrokeThickness 1, captioned, and differs only by Aspect —
//   - None:          the geometry is drawn at its natural size/offset, NOT stretched to the box;
//   - Fill:          stretched non-uniformly to fill the whole 100x100 box (the C# Stretch.Fill);
//   - Uniform:       scaled uniformly to fit inside the box (aspect ratio preserved, letterboxed);
//   - UniformToFill: scaled uniformly to cover the box (aspect ratio preserved, clipped).
//
// The page OWNS its whole element tree (the shapes_demo_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# source Data is the bird/duck-silhouette abbreviated-geometry string ("M8.0886959,0L…"),
//         one identical Path per Aspect. BUT the actual .NET MAUI Mac Catalyst render of this page
//         (verified against port/maui-reference/captures/maccatalyst/path_aspect_gallery_*) does NOT
//         show that silhouette at all — every cell renders as a plain RED OCTAGON with a YELLOW stroke
//         filling the gray box, identically across all four Aspect values (a MAUI-side rendering quirk
//         for this Path.Data content on this backend). The shared XAML twin independently documents and
//         reproduces the identical degraded silhouette via a Polygon octagon
//         ("4,50 26,12 50,4 74,12 96,50 74,88 50,96 26,88") inside a LightGray 100x100 Grid, since
//         Path.Data geometry authoring is unsupported by its loader. Per port/CLAUDE.md parity ruling 1,
//         MAUI's actual render is ground truth for page content, so the port reproduces the twin's
//         octagon/Grid structure directly instead of the mathematically-faithful (but NOT what MAUI
//         actually shows) bird-silhouette Path.
//   note: the C# <Style TargetType="Path"> sets HorizontalOptions="Start". The twin reproduces the
//         equivalent effect by giving each wrapping Grid HorizontalOptions="Start" (View.HorizontalOptions
//         -> set_horizontal_layout_alignment, honored at arrange time by LayoutExtensions.ComputeFrame);
//         the port mirrors that on each wrapping grid cell.
//   note: BackgroundColor="LightGray" (now on the wrapping Grid, matching the twin) →
//         set_background(solid_paint(colors::light_gray)); Stroke="Yellow" / Fill="Red" on the octagon →
//         solid_paint over colors::yellow / colors::red (the named-brush → paint bridge).
//   note: the Aspect distinction has no visible effect once the geometry is a plain fixed-size Polygon
//         (matching the twin's own note), so all four cells render the same resting shape; the four
//         separate Polygon+Grid instances are still built (rather than shared) to mirror the four
//         distinct twin elements.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class path_aspect_gallery_page
    {
    public:
        path_aspect_gallery_page()
        {
            page_.set_title("Path Aspect Gallery");
            stack_.set_padding(maui::core::thickness(12)); // C# StackLayout Padding="12"

            // --- None.
            none_label_.set_text("None");
            stack_.add(none_label_);
            style_cell(none_grid_, none_);
            stack_.add(none_grid_);

            // --- Fill.
            fill_label_.set_text("Fill");
            stack_.add(fill_label_);
            style_cell(fill_grid_, fill_);
            stack_.add(fill_grid_);

            // --- Uniform.
            uniform_label_.set_text("Uniform");
            stack_.add(uniform_label_);
            style_cell(uniform_grid_, uniform_);
            stack_.add(uniform_grid_);

            // --- UniformToFill.
            uniform_to_fill_label_.set_text("UniformToFill");
            stack_.add(uniform_to_fill_label_);
            style_cell(uniform_to_fill_grid_, uniform_to_fill_);
            stack_.add(uniform_to_fill_grid_);

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
        [[nodiscard]] maui::controls::shapes::polygon& none_path()
        {
            return none_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& fill_path()
        {
            return fill_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& uniform_path()
        {
            return uniform_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& uniform_to_fill_path()
        {
            return uniform_to_fill_;
        }

    private:
        // The shared octagon silhouette — MAUI's actual render of every cell on this page (header note).
        static maui::controls::shapes::point_collection octagon_points()
        {
            return {{4, 50}, {26, 12}, {50, 4}, {74, 12}, {96, 50}, {74, 88}, {50, 96}, {26, 88}};
        }

        // Configure one cell: a LightGray 100x100 Grid (Start-aligned, matching the twin) wrapping a
        // Yellow-stroked, Red-filled octagon Polygon.
        static void style_cell(maui::controls::grid& cell, maui::controls::shapes::polygon& shape)
        {
            cell.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::light_gray));
            cell.set_width_request(100);
            cell.set_height_request(100);
            cell.set_horizontal_layout_alignment(maui::core::layout_alignment::start);

            shape.set_points(octagon_points());
            shape.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));
            shape.set_fill(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            shape.set_stroke_thickness(1);
            cell.add(shape);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label none_label_;
        maui::controls::grid none_grid_;
        maui::controls::shapes::polygon none_;
        maui::controls::label fill_label_;
        maui::controls::grid fill_grid_;
        maui::controls::shapes::polygon fill_;
        maui::controls::label uniform_label_;
        maui::controls::grid uniform_grid_;
        maui::controls::shapes::polygon uniform_;
        maui::controls::label uniform_to_fill_label_;
        maui::controls::grid uniform_to_fill_grid_;
        maui::controls::shapes::polygon uniform_to_fill_;
    };
} // namespace maui::samples
