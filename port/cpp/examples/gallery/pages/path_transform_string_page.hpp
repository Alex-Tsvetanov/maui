#pragma once
// maui::samples::path_transform_string_page — ports PathTransformStringGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/PathTransformStringGallery.xaml: a ScrollView over a StackLayout
// (Padding 12) that shows the SAME two-figure Path geometry twice — once "Without RenderTransform"
// and once "With RenderTransform" — to contrast a transform-string-driven Path against a plain one.
// The shared geometry is a PathGeometry of two CLOSED figures (each a three-point triangle):
//   figure A: start (10,100) → LineSegment (100,100) → LineSegment (100,50), closed;
//   figure B: start (10,10)  → LineSegment (100,10)  → LineSegment (100,40), closed.
// The first Path carries RenderTransform="0.75 0 0 0.75 0 0" — the WPF-style six-number transform
// string (a uniform 0.75 scale about the origin). Both Paths take the implicit ResourceDictionary
// <Style TargetType="Path"> (100x100, black stroke, StrokeThickness 4).
//
// note: the C# captions read "Without RenderTransform" then "With RenderTransform". In the XAML the
//       FIRST (top) Path is the one that actually carries the RenderTransform string and the second is
//       plain — i.e. the captions sit ABOVE the Path they precede, so "Without RenderTransform" labels
//       the transformed Path and "With RenderTransform" labels the plain one. The port preserves the
//       XAML's literal element order: the transformed Path is added first (under the "Without
//       RenderTransform" caption), the plain Path second (under "With RenderTransform"), exactly as
//       authored — fidelity to the source tree over the label's intuitive reading.
//
// Demonstrated (all headless-safe maui:: shape API):
//   path::set_render_transform with a matrix_transform built from the parsed six-number transform
//   string ("0.75 0 0 0.75 0 0" → matrix{0.75,0,0,0.75,0,0}), contrasted against an identical untransformed
//   Path; two-figure PathGeometry authoring (path_figure + line_segment, IsClosed).
//
// The page OWNS its whole element tree (the path_gallery_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; the page is purely visual.
//   note: the C# RenderTransform="0.75 0 0 0.75 0 0" is the string form of a MatrixTransform (the WPF
//         "M11 M12 M21 M22 OffsetX OffsetY" order). The port builds the identical matrix_transform
//         directly (matrix{0.75,0,0,0.75,0,0}) — the resolved transform is the same; string→Transform
//         parsing is the XAML wave's job.
//   note: the two figures are authored as object-element PathGeometry/PathFigure/LineSegment in the
//         XAML; the port builds them programmatically from the same geometry objects (path_figure +
//         line_segment, set_is_closed(true)) — the resolved geometry is identical (the path_gallery_page
//         #2/#7 recipe).
//   note: the implicit <Style TargetType="Path"> (100x100, black Stroke, StrokeThickness 4) is inlined
//         on each Path; the C# Path Aspect is unset → defaults to None (no aspect fitting), so the port
//         leaves the default aspect untouched (faithful — the figures already sit within 100x100).

#include <memory>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/matrix_transform.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/path_figure.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_segment.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class path_transform_string_page
    {
    public:
        path_transform_string_page()
        {
            page_.set_title("Path Transform from string Gallery");
            stack_.set_padding(maui::core::thickness{12}); // C# StackLayout Padding="12"

            // --- "Without RenderTransform" caption + the transformed Path (RenderTransform 0.75 scale).
            without_label_.set_text("Without RenderTransform");
            stack_.add(without_label_);
            transformed_.set_data(build_two_figures());
            style_path(transformed_);
            {
                // RenderTransform="0.75 0 0 0.75 0 0" — the WPF six-number string as a MatrixTransform.
                auto transform = std::make_shared<maui::controls::shapes::matrix_transform>();
                transform->set_matrix(maui::controls::shapes::matrix{0.75, 0, 0, 0.75, 0, 0});
                transformed_.set_render_transform(std::move(transform));
            }
            stack_.add(transformed_);

            // --- "With RenderTransform" caption + the plain (untransformed) Path — same geometry.
            with_label_.set_text("With RenderTransform");
            stack_.add(with_label_);
            plain_.set_data(build_two_figures());
            style_path(plain_);
            stack_.add(plain_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::shapes::path& transformed()
        {
            return transformed_;
        }
        [[nodiscard]] maui::controls::shapes::path& plain()
        {
            return plain_;
        }

    private:
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        // Build the shared two-figure PathGeometry (each figure a CLOSED three-point triangle) — the
        // identical geometry the XAML authors object-element style under both Paths.
        static std::shared_ptr<maui::controls::shapes::path_geometry> build_two_figures()
        {
            auto geometry = std::make_shared<maui::controls::shapes::path_geometry>();

            // figure A: (10,100) → (100,100) → (100,50), closed.
            {
                auto figure = std::make_shared<maui::controls::shapes::path_figure>();
                figure->set_start_point(maui::graphics::point{10, 100});
                figure->set_is_closed(true);
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 100}));
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 50}));
                geometry->figures().push_back(std::move(figure));
            }
            // figure B: (10,10) → (100,10) → (100,40), closed.
            {
                auto figure = std::make_shared<maui::controls::shapes::path_figure>();
                figure->set_start_point(maui::graphics::point{10, 10});
                figure->set_is_closed(true);
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 10}));
                figure->segments().push_back(
                    std::make_shared<maui::controls::shapes::line_segment>(maui::graphics::point{100, 40}));
                geometry->figures().push_back(std::move(figure));
            }
            return geometry;
        }

        // Apply the implicit <Style TargetType="Path">: 100x100, black stroke, StrokeThickness 4.
        void style_path(maui::controls::shapes::path& target)
        {
            target.set_stroke(solid(maui::graphics::colors::black));
            target.set_stroke_thickness(4);
            target.set_width_request(100);
            target.set_height_request(100);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label without_label_;
        maui::controls::shapes::path transformed_;
        maui::controls::label with_label_;
        maui::controls::shapes::path plain_;
    };
} // namespace maui::samples
