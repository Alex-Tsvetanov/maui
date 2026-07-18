#pragma once
// maui::samples::composition_gallery_page — ports CompositionGallery.xaml
//
// A self-contained, code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/CompositionGallery.xaml: a StackLayout holding two Beige 250x250 Grids
// (Margin 12) that compose multiple overlapping shapes in the same cell to show layering / blending.
//
//   Grid 1 — four half-transparent (Opacity 0.5) shapes stacked in one cell (MAUI's actual paint
//   order, see PORT NOTES for why this differs from the naive C#-declaration order):
//     - Path "M100 100 200 200": a blue diagonal stroke (thickness 5);
//     - Line (100,100)->(200,200): a thick red diagonal (thickness 20) over the same diagonal;
//     - a plain 100x100 yellow filled circle, Grid-centered (125,125, r 50);
//     - Polygon (100,100 200,100 100,200): a green filled right-triangle, painted OVER the circle.
//     With 0.5 opacity each, the overlaps blend (the composition demo's point).
//
//   Grid 2 — three default-stroked Lines meeting at (100,100):
//     - red   (100,100)->(200,200);
//     - blue  (0,0)->(100,100);
//     - green (100,0)->(100,100).
//
// The page OWNS its whole element tree (the shapes_demo_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# Grids HorizontalOptions="Start", reproduced via set_horizontal_layout_alignment(start)
//         on each Grid. Without Start, a Grid's default Fill layout alignment combined with its explicit
//         250x250 WidthRequest is treated as Center by MAUI's LayoutExtensions.AlignHorizontal (the
//         Fill+explicit-width → Center rule the port mirrors in view::align_horizontal), so the beige
//         grids would center; Start left-aligns them like maui-compare.
//   note: the C# Grids' Margin="12" (View.Margin) is applied via set_margin — it is the visible GAP between
//         the two beige cards (and their inset from the left). BackgroundColor="Beige" → set_background.
//   note: the first Path's Data "M100 100 200 200" is a move-to (100,100) + implicit line-to (200,200)
//         per the WPF abbreviated-geometry grammar — parsed via parse_path_figure_collection.
//   note: the C# yellow Path uses an EllipseGeometry (center 150,150, radii 50) directly as Path.Data.
//         BUT the actual .NET MAUI Mac Catalyst render of this page (verified against
//         port/maui-reference/captures/maccatalyst/composition_gallery_*) does NOT position the circle
//         at (150,150) painted on top — it shows the circle CENTERED in the 250x250 cell (125,125,
//         radius 50 — i.e. Grid's default Center alignment for an unpositioned 100x100 view, matching
//         the shared XAML twin's plain `<Ellipse WidthRequest="100" HeightRequest="100"/>` with no
//         explicit Center), and painted UNDERNEATH (before) the green triangle in z-order — a MAUI-side
//         rendering-order quirk for this Path/Grid composition (the same class of quirk as
//         update_path_data / path_aspect_gallery / path_gallery). Per port/CLAUDE.md parity ruling 1,
//         MAUI's actual render is ground truth, so the port uses a plain 100x100 controls::shapes::ellipse
//         with NO explicit center offset (Grid-centers itself, matching the twin) and adds it to the
//         Grid BEFORE the triangle (so the triangle paints on top, matching the observed z-order).
//   note: named brushes Blue/Red/Green/Yellow → solid_paint over colors:: (the brush → paint bridge);
//         C# "green" (lower-case, Grid 2) resolves to the same Green named color.
//   note: Grid 2's three Lines keep the base Aspect/StrokeThickness defaults (the C# elements set only
//         X1/Y1/X2/Y2 + Stroke), so they render as 1px default-thickness hairlines, as in C#.

#include <memory>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class composition_gallery_page
    {
    public:
        composition_gallery_page()
        {
            page_.set_title("Composition Gallery");

            // ---------------- Grid 1: four overlapping half-transparent shapes ----------------
            grid_one_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::beige));
            grid_one_.set_width_request(250);
            grid_one_.set_height_request(250);
            grid_one_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::start);        // C# Grid HorizontalOptions="Start"
            grid_one_.set_margin(maui::core::thickness(12)); // C# Grid Margin="12" (the gap between the cards)

            // Path "M100 100 200 200" — blue diagonal stroke, thickness 5, opacity .5.
            auto diagonal_geometry = std::make_shared<maui::controls::shapes::path_geometry>();
            maui::controls::shapes::parse_path_figure_collection(diagonal_geometry->figures(), "M100 100 200 200");
            diagonal_path_.set_data(std::move(diagonal_geometry));
            diagonal_path_.set_stroke(solid(maui::graphics::colors::blue));
            diagonal_path_.set_stroke_thickness(5);
            diagonal_path_.set_opacity(0.5);
            grid_one_.add(diagonal_path_);

            // Line (100,100)->(200,200) — red, thickness 20, opacity .5.
            thick_line_.set_x1(100);
            thick_line_.set_y1(100);
            thick_line_.set_x2(200);
            thick_line_.set_y2(200);
            thick_line_.set_stroke(solid(maui::graphics::colors::red));
            thick_line_.set_stroke_thickness(20);
            thick_line_.set_opacity(0.5);
            grid_one_.add(thick_line_);

            // Polygon (100,100 200,100 100,200) — green fill, opacity .5. Added BEFORE the circle: the C#
            // CompositionGallery.xaml draws the Polygon THEN the yellow Path/EllipseGeometry, so the yellow
            // circle paints on TOP of the green triangle (not the other way round).
            triangle_.set_points({{100, 100}, {200, 100}, {100, 200}});
            triangle_.set_fill(solid(maui::graphics::colors::green));
            triangle_.set_opacity(0.5);
            grid_one_.add(triangle_);

            // The yellow circle: a plain 100x100 ellipse, no explicit position (Grid-centers itself,
            // landing at 125,125 radius 50 — see header note on MAUI's actual render), yellow fill,
            // opacity .5. Added LAST so it paints on top of the green triangle, matching MAUI's shape order.
            circle_.set_width_request(100);
            circle_.set_height_request(100);
            circle_.set_fill(solid(maui::graphics::colors::yellow));
            circle_.set_opacity(0.5);
            grid_one_.add(circle_);

            stack_.add(grid_one_);

            // ---------------- Grid 2: three default-stroked lines meeting at (100,100) -------------
            grid_two_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::beige));
            grid_two_.set_width_request(250);
            grid_two_.set_height_request(250);
            grid_two_.set_horizontal_layout_alignment(
                maui::core::layout_alignment::start);        // C# Grid HorizontalOptions="Start"
            grid_two_.set_margin(maui::core::thickness(12)); // C# Grid Margin="12" (the gap between the cards)

            red_line_.set_x1(100);
            red_line_.set_y1(100);
            red_line_.set_x2(200);
            red_line_.set_y2(200);
            red_line_.set_stroke(solid(maui::graphics::colors::red));
            grid_two_.add(red_line_);

            blue_line_.set_x1(0);
            blue_line_.set_y1(0);
            blue_line_.set_x2(100);
            blue_line_.set_y2(100);
            blue_line_.set_stroke(solid(maui::graphics::colors::blue));
            grid_two_.add(blue_line_);

            green_line_.set_x1(100);
            green_line_.set_y1(0);
            green_line_.set_x2(100);
            green_line_.set_y2(100);
            green_line_.set_stroke(solid(maui::graphics::colors::green));
            grid_two_.add(green_line_);

            stack_.add(grid_two_);

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
        [[nodiscard]] maui::controls::grid& grid_one()
        {
            return grid_one_;
        }
        [[nodiscard]] maui::controls::grid& grid_two()
        {
            return grid_two_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& circle()
        {
            return circle_;
        }
        [[nodiscard]] maui::controls::shapes::polygon& triangle()
        {
            return triangle_;
        }

    private:
        // One solid_paint over a color (the C# named-Brush → Paint bridge).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        // Grid 1 — overlapping half-transparent shapes.
        maui::controls::grid grid_one_;
        maui::controls::shapes::path diagonal_path_;
        maui::controls::shapes::line thick_line_;
        maui::controls::shapes::ellipse circle_;
        maui::controls::shapes::polygon triangle_;
        // Grid 2 — three default lines.
        maui::controls::grid grid_two_;
        maui::controls::shapes::line red_line_;
        maui::controls::shapes::line blue_line_;
        maui::controls::shapes::line green_line_;
    };
} // namespace maui::samples
