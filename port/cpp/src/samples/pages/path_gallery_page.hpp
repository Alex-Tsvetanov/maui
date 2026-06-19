#pragma once
// maui::samples::path_gallery_page — ports PathGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery Pages/Controls/ShapesGalleries/PathGallery.xaml:
// a ScrollView over a StackLayout (Padding 12) that walks eight Path variants (plus two caption-only
// markup-string Labels), each under a caption Label. Every Path in the C# page carries the implicit
// ResourceDictionary <Style TargetType="Path"> (Aspect=Uniform, HorizontalOptions=Start); the port
// applies aspect_fit (the documented Uniform→aspect_fit collapse, shape.hpp) to each.
//
//   - "Create a LineSegment in a PathGeometry": a single line from markup "M 10,50 L 200,70", black
//                                               stroke 1, 100x100;
//   - "Create a Shape by Using a PathGeometry": a CLOSED PathGeometry built programmatically — start
//                                               (10,100), LineSegment (100,100), LineSegment (100,50),
//                                               IsClosed (a triangle), black stroke 1, 100x100;
//   - "Cubic Bezier Path": one cubic from markup "M 10,100 C 100,0 200,200 300,100", black stroke 1;
//   - "Composite shape": a GeometryGroup (EvenOdd) of four CONCENTRIC EllipseGeometry (radii 50/70/100/
//                        120, all centered 75,75), #CCCCFF fill, black stroke 1, 120x120 — the EvenOdd
//                        rule renders the alternating rings hollow/filled;
//   - "Overlapping Rectangles": a GeometryGroup of two RectangleGeometry (480,96,192,192 and
//                               576,192,192,192), red fill + stroke 3, 100x100;
//   - "EllipseGeometry": a GeometryGroup of four EllipseGeometry (the 2x2 cluster of circles, radii 100),
//                        orange fill, green stroke 2, 100x100;
//   - "Multiple Line Segments": an OPEN PathGeometry — start (144,72) then four LineSegments — the classic
//                               self-intersecting star outline, aqua fill, maroon stroke 3, 200x200;
//   - "Complex Paths": the four-quadrant glyph markup, black stroke 1, 100x100; followed by the leaf glyph
//                      markup rendered twice over (a yellow-stroked red fill is the second copy).
//
// The page OWNS its whole element tree (the shapes_page / shapes_demo_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; the page is purely visual.
//   note: the markup-driven Paths (#1, #3, #8 + the two complex glyphs) parse their Data strings through
//         the port's path markup parser (parse_path_geometry ⇐ PathGeometryConverter), wrapped in a
//         shared_ptr<geometry> for Path.set_data — exactly the shapes_demo_page recipe.
//   note: the element-tree-authored geometries (#2 PathGeometry, #4/#5/#6 GeometryGroup, #7 PathGeometry)
//         are built programmatically from the same geometry objects the XAML names (PathFigure +
//         LineSegment, GeometryGroup + EllipseGeometry/RectangleGeometry), since object-element geometry
//         authoring is the XAML wave's job — the resolved geometry is identical to the markup tree.
//   note: the C# <Style TargetType="Path"> (Aspect=Uniform, HorizontalOptions=Start) is a XAML resource
//         style applied to every Path; the port inlines Aspect via aspect_fit on each path
//         (Uniform→aspect_fit, shape.hpp). HorizontalOptions=Start is a layout-option the port does not
//         model on shapes today, so it is omitted (best-effort).
//   note: the two caption-only Labels in the C# page that show the raw glyph markup strings (FontSize 9)
//         are reproduced as caption labels so the page reads the same; their long Data strings are the
//         very strings fed to the matching Paths below them.
//   note: fill/stroke colors are named brushes ("Black", "Red", "Orange", "Green", "Aqua", "Maroon",
//         "Yellow") except #CCCCFF (the Composite shape fill), reconstructed via color::from_argb — the
//         cross-platform equivalent of the named/literal brush. Each is wrapped in a solid_paint (the
//         documented brush→paint bridge).
//   note: the C# StackLayout Padding="12" is not modeled on this layout in the port today, so it is
//         omitted (best-effort; the eight paths and their order are what the page demonstrates).

#include <memory>
#include <string_view>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/geometry_group.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/path_segment.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/path_aspect.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class path_gallery_page
    {
    public:
        // The two complex glyph markup strings, shared between the caption labels and the paths below them
        // (the C# page shows each string in a FontSize=9 Label, then feeds the same string to a Path Data).
        static constexpr std::string_view k_four_quadrant_data =
            "M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z "
            "M0,16.207977L11.904009,16.207977 11.904009,29.900984 0,28.657984z "
            "M11.904036,2.0979624L11.904036,14.202982 2.7656555E-05,14.202982 2.7656555E-05,3.3409645z "
            "M32.000058,0L32.000058,14.203001 13.909059,14.203001 13.909059,1.8890382z";
        static constexpr std::string_view k_leaf_glyph_data =
            "M8.0886959,0L8.687694,0C12.279728,0.2989963 14.275696,2.2949993 15.971676,4.9890003 "
            "16.271724,4.5899982 16.470699,4.1909961 16.670711,3.8920001 18.765678,0.89799553 "
            "23.056684,-1.0980074 27.247655,0.79800445 28.544711,1.3970038 29.842683,2.2949993 "
            "30.740692,3.5919966 31.239652,4.3909931 31.837675,5.6880059 31.93765,6.8849973 "
            "32.336696,10.677006 30.740692,13.470998 29.442659,15.866003L26.648658,15.866003C26.149696,"
            "15.168005 26.050697,14.069998 25.351663,13.571004 24.453716,14.369009 24.353679,15.966009 "
            "23.75572,17.064001 23.156721,17.263006 22.457687,16.96401 21.759691,17.163 21.260667,"
            "17.761999 20.960681,19.359001 20.761707,20.257011 20.761707,19.458 20.561695,17.761999 "
            "20.462695,16.664007 20.262683,14.668997 20.162708,12.472997 19.963674,10.278004 19.863698,"
            "9.3800086 19.963674,8.1830015 19.164724,8.1830015 18.566703,8.1830015 18.466728,9.3800086 "
            "18.466728,9.9790077 18.266715,12.07401 17.867731,14.27001 17.468683,16.465002 16.969722,"
            "15.467001 16.670711,14.27001 16.171687,14.27001 15.57269,14.668997 15.27368,15.36701 "
            "14.973692,15.966009L13.975708,15.966009C13.876709,15.666998 13.576723,15.567007 13.277712,"
            "15.567007 12.878725,15.567007 12.47974,15.966009 12.47974,16.465002 12.47974,16.96401 "
            "12.878725,17.362997 13.277712,17.362997 13.476686,17.362997 13.776735,17.263006 13.876709,"
            "17.064001 14.375732,17.163 15.073729,17.064001 15.57269,17.064001 15.871701,16.664007 "
            "15.971676,16.265005 16.171687,15.966009 16.76971,16.763998 16.670711,18.161003 17.767694,"
            "18.660011 18.166679,18.361 18.266715,17.961998 18.366691,17.463003 18.566703,16.066 "
            "18.865714,14.569006 19.065725,13.071996 19.065725,12.873006 19.164724,11.675008 19.264699,"
            "11.375997 19.464712,14.069998 19.763723,17.761999 19.963674,20.556007 20.062671,21.354011 "
            "20.262683,21.554008 20.861682,21.953011 21.360704,21.554008 21.459703,21.454002 21.659715,"
            "20.855003 21.958665,20.157005 22.0587,19.359001 22.258712,18.560005 22.757675,18.461006 "
            "23.75572,18.760002 24.353679,18.461006 24.852703,17.662008 25.052713,16.364996 25.4517,"
            "15.567007 25.750711,16.066 25.950662,16.763998 26.249671,17.163L28.844699,17.163C28.445651,"
            "17.761999 27.846654,18.361 27.447667,18.760002 24.253703,22.352013 20.162708,25.545008 "
            "16.071712,27.641001 10.982733,24.84701 5.6937417,20.955009 2.4007186,15.567007 0.90371192,"
            "13.071996 -0.79226869,8.9810066 0.40475065,5.3889946 0.60476232,4.8900012 0.90371192,"
            "4.4899921 1.2037603,3.9909992 2.4007183,1.7959909 5.0947441,2.1702817E-07 8.0886959,0z";

        path_gallery_page()
        {
            page_.set_title("Path Gallery");

            // --- #1 "Create a LineSegment in a PathGeometry": one line from markup, black stroke 1, 100x100.
            caption(line_seg_label_, "Create a LineSegment in a PathGeometry");
            line_seg_.set_data(parse(std::string_view{"M 10,50 L 200,70"}));
            stroke_path(line_seg_, maui::graphics::colors::black, 1);
            line_seg_.set_width_request(100);
            line_seg_.set_height_request(100);
            stack_.add(line_seg_);

            // --- #2 "Create a Shape by Using a PathGeometry": a CLOSED triangle built programmatically.
            caption(geom_label_, "Create a Shape by Using a PathGeometry");
            {
                auto figure = std::make_shared<maui::controls::shapes::path_figure>();
                figure->set_start_point(maui::graphics::point{10, 100});
                figure->set_is_closed(true);
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 100}));
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 50}));
                auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();
                geometry->figures().push_back(std::move(figure));
                geom_.set_data(std::move(geometry));
            }
            stroke_path(geom_, maui::graphics::colors::black, 1);
            geom_.set_width_request(100);
            geom_.set_height_request(100);
            stack_.add(geom_);

            // --- #3 "Cubic Bezier Path": one cubic from markup, black stroke 1, 100x100.
            caption(cubic_label_, "Cubic Bezier Path");
            cubic_.set_data(parse(std::string_view{"M 10,100 C 100,0 200,200 300,100"}));
            stroke_path(cubic_, maui::graphics::colors::black, 1);
            cubic_.set_width_request(100);
            cubic_.set_height_request(100);
            stack_.add(cubic_);

            // --- #4 "Composite shape": GeometryGroup (EvenOdd) of four concentric ellipses, #CCCCFF fill,
            //     black stroke 1, 120x120.
            caption(composite_label_, "Composite shape");
            {
                auto group = std::make_shared<maui::controls::shapes::geometry_group>();
                group->set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
                for (const double radius : {50.0, 70.0, 100.0, 120.0})
                {
                    group->children().push_back(std::make_shared<maui::controls::shapes::ellipse_geometry>(
                        maui::graphics::point{75, 75}, radius, radius));
                }
                composite_.set_data(std::move(group));
            }
            composite_.set_fill(solid(maui::graphics::color::from_argb("#CCCCFF")));
            stroke_path(composite_, maui::graphics::colors::black, 1);
            composite_.set_width_request(120);
            composite_.set_height_request(120);
            stack_.add(composite_);

            // --- #5 "Overlapping Rectangles": GeometryGroup of two rectangles, red fill + stroke 3, 100x100.
            caption(rects_label_, "Overlapping Rectangles");
            {
                auto group = std::make_shared<maui::controls::shapes::geometry_group>();
                group->children().push_back(std::make_shared<maui::controls::shapes::rectangle_geometry>(
                    maui::graphics::rect{480, 96, 192, 192}));
                group->children().push_back(std::make_shared<maui::controls::shapes::rectangle_geometry>(
                    maui::graphics::rect{576, 192, 192, 192}));
                rects_.set_data(std::move(group));
            }
            rects_.set_fill(solid(maui::graphics::colors::red));
            stroke_path(rects_, maui::graphics::colors::red, 3);
            rects_.set_width_request(100);
            rects_.set_height_request(100);
            stack_.add(rects_);

            // --- #6 "EllipseGeometry": GeometryGroup of four circles (2x2 cluster), orange fill, green
            //     stroke 2, 100x100.
            caption(ellipses_label_, "EllipseGeometry");
            {
                auto group = std::make_shared<maui::controls::shapes::geometry_group>();
                for (const maui::graphics::point center :
                     {maui::graphics::point{150, 150}, maui::graphics::point{250, 150}, maui::graphics::point{150, 250},
                      maui::graphics::point{250, 250}})
                {
                    group->children().push_back(
                        std::make_shared<maui::controls::shapes::ellipse_geometry>(center, 100, 100));
                }
                ellipses_.set_data(std::move(group));
            }
            ellipses_.set_fill(solid(maui::graphics::colors::orange));
            stroke_path(ellipses_, maui::graphics::colors::green, 2);
            ellipses_.set_width_request(100);
            ellipses_.set_height_request(100);
            stack_.add(ellipses_);

            // --- #7 "Multiple Line Segments": an OPEN PathGeometry, start (144,72) + four LineSegments —
            //     the classic self-intersecting star outline, aqua fill, maroon stroke 3, 200x200.
            caption(multi_seg_label_, "Multiple Line Segments");
            {
                auto figure = std::make_shared<maui::controls::shapes::path_figure>();
                figure->set_start_point(maui::graphics::point{144, 72});
                for (const maui::graphics::point point :
                     {maui::graphics::point{200, 246}, maui::graphics::point{53, 138}, maui::graphics::point{235, 138},
                      maui::graphics::point{88, 246}})
                {
                    figure->segments().push_back(std::make_shared<maui::controls::shapes::line_segment>(point));
                }
                auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();
                geometry->figures().push_back(std::move(figure));
                multi_seg_.set_data(std::move(geometry));
            }
            multi_seg_.set_fill(solid(maui::graphics::colors::aqua));
            stroke_path(multi_seg_, maui::graphics::colors::maroon, 3);
            multi_seg_.set_width_request(200);
            multi_seg_.set_height_request(200);
            stack_.add(multi_seg_);

            // --- #8 "Complex Paths": the two glyph markup strings (each shown first in a FontSize=9 caption,
            //     then drawn): the four-quadrant glyph (black stroke 1) and the leaf glyph (yellow stroke 1,
            //     red fill), each 100x100.
            caption(complex_label_, "Complex Paths");
            caption(four_quadrant_markup_label_, k_four_quadrant_data); // the FontSize=9 markup-string label
            four_quadrant_.set_data(parse(k_four_quadrant_data));
            stroke_path(four_quadrant_, maui::graphics::colors::black, 1);
            four_quadrant_.set_width_request(100);
            four_quadrant_.set_height_request(100);
            stack_.add(four_quadrant_);

            caption(leaf_markup_label_, k_leaf_glyph_data); // the second FontSize=9 markup-string label
            leaf_.set_data(parse(k_leaf_glyph_data));
            leaf_.set_fill(solid(maui::graphics::colors::red));
            stroke_path(leaf_, maui::graphics::colors::yellow, 1);
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

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children first in add()-order, then
        // the stack, then the scroll_view, then the page), then re-host the tree built in the ctor
        // (gallery_attach.hpp). The generic lambda preserves each member's concrete static type.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, line_seg_label_, "line_seg_label_");
            gallery_attach_one(app, line_seg_, "line_seg_");
            gallery_attach_one(app, geom_label_, "geom_label_");
            gallery_attach_one(app, geom_, "geom_");
            gallery_attach_one(app, cubic_label_, "cubic_label_");
            gallery_attach_one(app, cubic_, "cubic_");
            gallery_attach_one(app, composite_label_, "composite_label_");
            gallery_attach_one(app, composite_, "composite_");
            gallery_attach_one(app, rects_label_, "rects_label_");
            gallery_attach_one(app, rects_, "rects_");
            gallery_attach_one(app, ellipses_label_, "ellipses_label_");
            gallery_attach_one(app, ellipses_, "ellipses_");
            gallery_attach_one(app, multi_seg_label_, "multi_seg_label_");
            gallery_attach_one(app, multi_seg_, "multi_seg_");
            gallery_attach_one(app, complex_label_, "complex_label_");
            gallery_attach_one(app, four_quadrant_markup_label_, "four_quadrant_markup_label_");
            gallery_attach_one(app, four_quadrant_, "four_quadrant_");
            gallery_attach_one(app, leaf_markup_label_, "leaf_markup_label_");
            gallery_attach_one(app, leaf_, "leaf_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroll_, "scroll_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);   // stack hosts every path + caption
            gallery_rehost_content(scroll_); // scroll hosts the stack
            gallery_rehost_content(page_);   // page hosts the scroll
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
        [[nodiscard]] maui::controls::shapes::path& line_segment_path()
        {
            return line_seg_;
        }
        [[nodiscard]] maui::controls::shapes::path& composite()
        {
            return composite_;
        }
        [[nodiscard]] maui::controls::shapes::path& leaf()
        {
            return leaf_;
        }

    private:
        // Parse a Data markup string into an owned geometry for Path.set_data (the shapes_demo_page recipe).
        static std::shared_ptr<maui::controls::shapes::geometry> parse(std::string_view markup)
        {
            return std::make_shared<maui::controls::shapes::path_geometry>(
                maui::controls::shapes::parse_path_geometry(markup));
        }
        // One solid_paint over a color (the C# Brush→Paint bridge for a named/literal fill or stroke).
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }
        // Apply the shared per-path stroke + the implicit-style Aspect=Uniform (→aspect_fit, shape.hpp).
        static void stroke_path(maui::controls::shapes::path& path, maui::graphics::color stroke, double thickness)
        {
            path.set_stroke(solid(stroke));
            path.set_stroke_thickness(thickness);
            path.set_aspect(maui::core::path_aspect::aspect_fit); // C# <Style> Aspect="Uniform"
        }
        // One caption label above a path (used both for the section captions and the FontSize=9 markup
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
        maui::controls::shapes::path line_seg_;
        maui::controls::label geom_label_;
        maui::controls::shapes::path geom_;
        maui::controls::label cubic_label_;
        maui::controls::shapes::path cubic_;
        maui::controls::label composite_label_;
        maui::controls::shapes::path composite_;
        maui::controls::label rects_label_;
        maui::controls::shapes::path rects_;
        maui::controls::label ellipses_label_;
        maui::controls::shapes::path ellipses_;
        maui::controls::label multi_seg_label_;
        maui::controls::shapes::path multi_seg_;
        maui::controls::label complex_label_;
        maui::controls::label four_quadrant_markup_label_;
        maui::controls::shapes::path four_quadrant_;
        maui::controls::label leaf_markup_label_;
        maui::controls::shapes::path leaf_;
    };
} // namespace maui::samples
