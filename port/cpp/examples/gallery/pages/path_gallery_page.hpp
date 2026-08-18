#pragma once
// maui::samples::path_gallery_page — ports PathGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml:
// a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only
// markup-string Labels), each under a caption Label.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# source authors every cell as a genuine <Path Data="…"/> (or element-tree geometry).
//         BUT the actual .NET MAUI Mac Catalyst render of this page (verified against
//         port/maui-reference/captures/maccatalyst/path_gallery_*) does NOT show that geometry content
//         for several cells — it is a MAUI-side rendering quirk for certain Path.Data content on this
//         backend (the same class of quirk as update_path_data / path_aspect_gallery). The shared XAML
//         twin independently documents and reproduces the identical degraded silhouettes (its loader
//         also lacks Path.Data geometry authoring), and matches the real MAUI capture pixel-for-pixel
//         (sonnet_xaml: green). Per port/CLAUDE.md parity ruling 1, MAUI's actual render is ground truth
//         for page content, so the port aligns every cell to the twin's rendered shape:
//           - "Create a LineSegment in a PathGeometry": a Line (10,50)->(200,70), black stroke 1 — MATCHES
//             MAUI already (unchanged).
//           - "Create a Shape by Using a PathGeometry": a closed triangle Polygon (10,100 100,100 100,50),
//             black stroke 1 — MATCHES MAUI already (unchanged).
//           - "Cubic Bezier Path": MAUI renders a zig-zag through the curve's sampled points, not a
//             smooth curve — a Polyline "10,100 85,45 155,100 225,155 300,100", black stroke 1.
//           - "Composite shape": MAUI renders a single plain filled circle (NOT concentric rings) — a
//             single Ellipse, #CCCCFF fill, black stroke 1, 120x120.
//           - "Overlapping Rectangles": MAUI renders a single solid red square (NOT two overlapping
//             even-odd rects) — a single Rectangle, red fill + stroke 3, 100x100.
//           - "EllipseGeometry": MAUI renders a single circle (NOT a 2x2 cluster) — a single Ellipse,
//             orange fill, green stroke 2, 100x100.
//           - "Multiple Line Segments": MAUI renders an UNFILLED dark-red star OUTLINE (NOT filled aqua)
//             — a Polyline through the same five points, maroon stroke 3, no fill.
//           - "Complex Paths": MAUI renders plain placeholder shapes for the two glyph markups (NOT the
//             actual glyph geometry) — a bordered Rectangle placeholder (black stroke 1, 100x100) for the
//             four-quadrant glyph, and a red-filled/yellow-stroked Ellipse placeholder (100x100) for the
//             leaf glyph. The two caption Labels showing the raw glyph markup strings carry FontSize 9;
//             the second is ABBREVIATED to the twin's stand-in (the twin, not the C# sample, is what
//             the maui ground-truth column renders).
//   note: the C# <Style TargetType="Path"> (Aspect=Uniform, HorizontalOptions=Start) applied only to
//         genuine Path elements; since every cell is now a non-Path shape (Line/Polygon/Polyline/
//         Ellipse/Rectangle), this styling has no remaining Path target and is dropped, matching the
//         twin (which does not reproduce it either — the degraded shapes carry their own fixed size).
//   note: fill/stroke colors are named brushes ("Black", "Red", "Orange", "Green", "Maroon") except
//         #CCCCFF (the Composite shape fill), reconstructed via color::from_argb — the cross-platform
//         equivalent of the named/literal brush. Each is wrapped in a solid_paint (the documented
//         brush→paint bridge).
//   note: the C# StackLayout Padding="12" IS reproduced: PORT FIX (2026-07-06, the cpp<->xaml consistency
//         check) — the twin (pages/path_gallery.xaml) DOES carry <VerticalStackLayout Padding="12">; a
//         stale comment here previously claimed the twin also omitted it, which was simply wrong (verified
//         by reading the twin XAML directly). Without the matching padding, every shape sat flush against
//         the left edge (padding 0) while the twin's hydration correctly inset everything by 12pt,
//         producing a uniform ~18px native horizontal offset between the cpp and xaml captures despite
//         otherwise-identical content — exactly the kind of divergence the consistency check exists to
//         catch. stack_.set_padding(12) below now matches.

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class path_gallery_page
    {
    public:
        // The two complex glyph markup strings, shown verbatim in the FontSize=9 caption Labels above
        // their (placeholder) shapes — the C# page shows each raw Data string, then the shape below it.
        static constexpr std::string_view k_four_quadrant_data =
            "M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z "
            "M0,16.207977L11.904009,16.207977 11.904009,29.900984 0,28.657984z "
            "M11.904036,2.0979624L11.904036,14.202982 2.7656555E-05,14.202982 2.7656555E-05,3.3409645z "
            "M32.000058,0L32.000058,14.203001 13.909059,14.203001 13.909059,1.8890382z";
        // ABBREVIATED, matching the twin (pages/path_gallery.xaml:53) rather than the full ~2440-char
        // C# sample string. The twin is what the maui ground-truth column renders; the full string
        // wraps to ~13 lines at FontSize 9 and made this page ~174px taller than that column.
        static constexpr std::string_view k_leaf_glyph_data =
            "M8.0886959,0L8.687694,0C12.279728,0.2989963 14.275696,2.2949993 15.971676,4.9890003 ... "
            "8.0886959,0z";

        path_gallery_page()
        {
            page_.set_title("Path Gallery");
            stack_.set_padding(maui::core::thickness{12}); // StackLayout Padding="12" (see header note)

            // --- #1 "Create a LineSegment in a PathGeometry": a Line, black stroke 1 — matches MAUI.
            caption(line_seg_label_, "Create a LineSegment in a PathGeometry");
            line_seg_.set_x1(10);
            line_seg_.set_y1(50);
            line_seg_.set_x2(200);
            line_seg_.set_y2(70);
            line_seg_.set_stroke(solid(maui::graphics::colors::black));
            line_seg_.set_stroke_thickness(1);
            stack_.add(line_seg_);

            // --- #2 "Create a Shape by Using a PathGeometry": a closed triangle Polygon — matches MAUI.
            caption(geom_label_, "Create a Shape by Using a PathGeometry");
            geom_.set_points({{10, 100}, {100, 100}, {100, 50}});
            geom_.set_stroke(solid(maui::graphics::colors::black));
            geom_.set_stroke_thickness(1);
            stack_.add(geom_);

            // --- #3 "Cubic Bezier Path": MAUI renders a zig-zag through the curve's sampled points.
            caption(cubic_label_, "Cubic Bezier Path");
            cubic_.set_points({{10, 100}, {85, 45}, {155, 100}, {225, 155}, {300, 100}});
            cubic_.set_stroke(solid(maui::graphics::colors::black));
            cubic_.set_stroke_thickness(1);
            stack_.add(cubic_);

            // --- #4 "Composite shape": MAUI renders a single plain filled circle, #CCCCFF fill, black
            //     stroke 1, 120x120.
            caption(composite_label_, "Composite shape");
            composite_.set_fill(solid(maui::graphics::color::from_argb("#CCCCFF")));
            composite_.set_stroke(solid(maui::graphics::colors::black));
            composite_.set_stroke_thickness(1);
            composite_.set_width_request(120);
            composite_.set_height_request(120);
            stack_.add(composite_);

            // --- #5 "Overlapping Rectangles": MAUI renders a single solid red square, red fill + stroke
            //     3, 100x100.
            caption(rects_label_, "Overlapping Rectangles");
            rects_.set_fill(solid(maui::graphics::colors::red));
            rects_.set_stroke(solid(maui::graphics::colors::red));
            rects_.set_stroke_thickness(3);
            rects_.set_width_request(100);
            rects_.set_height_request(100);
            stack_.add(rects_);

            // --- #6 "EllipseGeometry": MAUI renders a single circle, orange fill, green stroke 2,
            //     100x100.
            caption(ellipses_label_, "EllipseGeometry");
            ellipses_.set_fill(solid(maui::graphics::colors::orange));
            ellipses_.set_stroke(solid(maui::graphics::colors::green));
            ellipses_.set_stroke_thickness(2);
            ellipses_.set_width_request(100);
            ellipses_.set_height_request(100);
            stack_.add(ellipses_);

            // --- #7 "Multiple Line Segments": MAUI renders an UNFILLED dark-red star outline, maroon
            //     stroke 3, no fill.
            caption(multi_seg_label_, "Multiple Line Segments");
            multi_seg_.set_points({{144, 72}, {200, 246}, {53, 138}, {235, 138}, {88, 246}});
            multi_seg_.set_stroke(solid(maui::graphics::colors::maroon));
            multi_seg_.set_stroke_thickness(3);
            stack_.add(multi_seg_);

            // --- #8 "Complex Paths": MAUI renders plain placeholder shapes for the two glyph markups.
            caption(complex_label_, "Complex Paths");
            four_quadrant_markup_label_.set_font(maui::core::font::system_font_of_size(9.0));
            caption(four_quadrant_markup_label_, k_four_quadrant_data); // the FontSize=9 markup-string label
            four_quadrant_.set_stroke(solid(maui::graphics::colors::black));
            four_quadrant_.set_stroke_thickness(1);
            four_quadrant_.set_width_request(100);
            four_quadrant_.set_height_request(100);
            stack_.add(four_quadrant_);

            leaf_markup_label_.set_font(maui::core::font::system_font_of_size(9.0));
            caption(leaf_markup_label_, k_leaf_glyph_data); // the second FontSize=9 markup-string label
            leaf_.set_fill(solid(maui::graphics::colors::red));
            leaf_.set_stroke(solid(maui::graphics::colors::yellow));
            leaf_.set_stroke_thickness(1);
            leaf_.set_width_request(100);
            leaf_.set_height_request(100);
            stack_.add(leaf_);

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
        [[nodiscard]] maui::controls::shapes::line& line_segment_path()
        {
            return line_seg_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& composite()
        {
            return composite_;
        }
        [[nodiscard]] maui::controls::shapes::ellipse& leaf()
        {
            return leaf_;
        }

    private:
        // One solid_paint over a color (the C# Brush→Paint bridge for a named/literal fill or stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // One caption label above a shape (used both for the section captions and the FontSize=9 markup
        // strings the C# page surfaces above the two complex paths).
        void caption(maui::controls::label& text, std::string_view value)
        {
            text.set_text(std::string{value});
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label line_seg_label_;
        maui::controls::shapes::line line_seg_;
        maui::controls::label geom_label_;
        maui::controls::shapes::polygon geom_;
        maui::controls::label cubic_label_;
        maui::controls::shapes::polyline cubic_;
        maui::controls::label composite_label_;
        maui::controls::shapes::ellipse composite_;
        maui::controls::label rects_label_;
        maui::controls::shapes::rectangle rects_;
        maui::controls::label ellipses_label_;
        maui::controls::shapes::ellipse ellipses_;
        maui::controls::label multi_seg_label_;
        maui::controls::shapes::polyline multi_seg_;
        maui::controls::label complex_label_;
        maui::controls::label four_quadrant_markup_label_;
        maui::controls::shapes::rectangle four_quadrant_;
        maui::controls::label leaf_markup_label_;
        maui::controls::shapes::ellipse leaf_;
    };
} // namespace maui::samples
