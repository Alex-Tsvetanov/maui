#pragma once
// maui::samples::clip_gallery_page — ports ClipGallery.xaml
//
// The C# page (Pages/Controls/ShapesGalleries/ClipGallery.xaml; its .xaml.cs is an empty
// InitializeComponent) is a ScrollView over a StackLayout (Padding=12) that shows the SAME "oasis.jpg"
// image SEVEN times — one bare copy followed by six copies, each carrying a different geometry on its
// Image.Clip (the VisualElement.Clip / IView.Clip surface), each captioned by a Small-font label:
//   - the bare image (no clip),
//   - a RectangleGeometry      Rect="0, 15, 150, 150",
//   - a RoundRectangleGeometry CornerRadius="6" Rect="0, 15, 150, 150",
//   - an EllipseGeometry       Center="100, 100" RadiusX="100" RadiusY="100",
//   - a GeometryGroup          FillRule="EvenOdd" of four overlapping radius-100 ellipses, and
//   - a PathGeometry           Figures="M8 148 L156 148 L132 12 Z" (a triangle).
//
// The XAML <Style TargetType="Image"> sets BackgroundColor="LightGray", Aspect="AspectFill",
// HorizontalOptions="Start", HeightRequest=200, WidthRequest=200 — applied to every image below. The
// <Style TargetType="Label"> sets FontSize="Small"; the headless label surface has no FontSize setter
// here, so the caption text is what is reproduced (the small-font polish is best-effort).
//
// PORT MAPPING (every clip geometry is a maui::controls::shapes::geometry, and geometry : i_shape — see
// controls/shapes/geometry.hpp — so each ports 1:1 to view::set_clip(shared_ptr<i_shape>)):
//   - RectangleGeometry(Rect)               -> shapes::rectangle_geometry(rect{0,15,150,150})
//   - RoundRectangleGeometry(CR=6, Rect)    -> shapes::round_rectangle_geometry(corner_radius{6}, rect{0,15,150,150})
//   - EllipseGeometry(Center,Rx,Ry)         -> shapes::ellipse_geometry(point{100,100}, 100, 100)
//   - GeometryGroup{ EllipseGeometry… }     -> shapes::geometry_group (FillRule even_odd, four ellipse children)
//   - PathGeometry(Figures="…")             -> shapes::path_geometry, figures parsed from the markup
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree (the generic mount in app_host.hpp
// attaches every owned view's handler and hosts the tree). These are
// VISUAL clip demos: the clipped images render natively on macOS+iOS once a backend draws pixels.
//
// note: the C# Source="oasis.jpg" is a file image source; the headless gallery loads no pixels, so each
//       image carries a best-effort file_image_source("oasis.jpg") — the demonstrated feature is the
//       Clip geometry, not the bitmap. The Style's LightGray background + AspectFill + 200x200 are
//       applied so the clipped region is observable in a backend that does render.

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/ellipse_geometry.hpp"
#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/geometry_group.hpp"
#include "maui/controls/shapes/path_geometry.hpp"
#include "maui/controls/shapes/path_markup_parser.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/shapes/round_rectangle_geometry.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class clip_gallery_page
    {
    public:
        clip_gallery_page()
        {
            page_.set_title("Clip Gallery");
            stack_.set_padding(maui::core::thickness{12}); // StackLayout Padding="12"
            stack_.set_spacing(6);

            // ---- the six clip geometries, built so each can be applied to its captioned image ----
            // RectangleGeometry Rect="0, 15, 150, 150".
            rect_clip_ =
                std::make_shared<maui::controls::shapes::rectangle_geometry>(maui::graphics::rect{0, 15, 150, 150});

            // RoundRectangleGeometry CornerRadius="6" Rect="0, 15, 150, 150".
            round_rect_clip_ = std::make_shared<maui::controls::shapes::round_rectangle_geometry>(
                maui::graphics::corner_radius{6}, maui::graphics::rect{0, 15, 150, 150});

            // EllipseGeometry Center="100, 100" RadiusX="100" RadiusY="100".
            ellipse_clip_ =
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{100, 100}, 100, 100);

            // GeometryGroup FillRule="EvenOdd" of four overlapping radius-100 ellipses.
            group_clip_ = std::make_shared<maui::controls::shapes::geometry_group>();
            group_clip_->set_fill_rule(maui::controls::shapes::fill_rule::even_odd);
            group_clip_->children().push_back(
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{150, 150}, 100, 100));
            group_clip_->children().push_back(
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{250, 150}, 100, 100));
            group_clip_->children().push_back(
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{150, 250}, 100, 100));
            group_clip_->children().push_back(
                std::make_shared<maui::controls::shapes::ellipse_geometry>(maui::graphics::point{250, 250}, 100, 100));

            // PathGeometry Figures="M8 148 L156 148 L132 12 Z" (a triangle).
            path_clip_ = std::make_shared<maui::controls::shapes::path_geometry>();
            maui::controls::shapes::parse_path_figure_collection(path_clip_->figures(), "M8 148 L156 148 L132 12 Z");

            // ---- the seven captioned images (the first bare, the rest each carrying one clip) ----
            caption(image_label_, "Image");
            style_image(image_);
            stack_.add(image_);

            caption(rect_label_, "Clipped Image using RectangleGeometry");
            style_image(rect_image_);
            rect_image_.set_clip(rect_clip_);
            stack_.add(rect_image_);

            caption(round_rect_label_, "Clipped Image using RoundRectangleGeometry");
            style_image(round_rect_image_);
            round_rect_image_.set_clip(round_rect_clip_);
            stack_.add(round_rect_image_);

            caption(ellipse_label_, "Clipped Image using EllipseGeometry");
            style_image(ellipse_image_);
            ellipse_image_.set_clip(ellipse_clip_);
            stack_.add(ellipse_image_);

            caption(group_label_, "Clipped Image using GeometryGroup");
            style_image(group_image_);
            group_image_.set_clip(group_clip_);
            stack_.add(group_image_);

            caption(path_label_, "Clipped Image using PathGeometry");
            style_image(path_image_);
            path_image_.set_clip(path_clip_);
            stack_.add(path_image_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Owned controls exposed for the hosting main / inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::image& rect_image()
        {
            return rect_image_;
        }
        [[nodiscard]] maui::controls::image& round_rect_image()
        {
            return round_rect_image_;
        }
        [[nodiscard]] maui::controls::image& ellipse_image()
        {
            return ellipse_image_;
        }
        [[nodiscard]] maui::controls::image& group_image()
        {
            return group_image_;
        }
        [[nodiscard]] maui::controls::image& path_image()
        {
            return path_image_;
        }

    private:
        // The XAML <Style TargetType="Image">: LightGray background, AspectFill, 200x200, Start alignment
        // (the source bitmap is best-effort; see header).
        static void style_image(maui::controls::image& picture)
        {
            picture.set_source(std::make_shared<maui::controls::file_image_source>("oasis.jpg"));
            picture.set_aspect(maui::core::aspect::aspect_fill);
            picture.set_width_request(200);
            picture.set_height_request(200);
            picture.set_background(std::make_shared<maui::graphics::solid_paint>(
                maui::graphics::color::from_rgb(211, 211, 211))); // LightGray = #D3D3D3
        }

        // One Small-font caption label above an image (FontSize="Small" is best-effort; see header).
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label image_label_;
        maui::controls::image image_;
        maui::controls::label rect_label_;
        maui::controls::image rect_image_;
        maui::controls::label round_rect_label_;
        maui::controls::image round_rect_image_;
        maui::controls::label ellipse_label_;
        maui::controls::image ellipse_image_;
        maui::controls::label group_label_;
        maui::controls::image group_image_;
        maui::controls::label path_label_;
        maui::controls::image path_image_;

        // The six clip geometries (kept so the demo owns them while the images reference them).
        std::shared_ptr<maui::controls::shapes::rectangle_geometry> rect_clip_;
        std::shared_ptr<maui::controls::shapes::round_rectangle_geometry> round_rect_clip_;
        std::shared_ptr<maui::controls::shapes::ellipse_geometry> ellipse_clip_;
        std::shared_ptr<maui::controls::shapes::geometry_group> group_clip_;
        std::shared_ptr<maui::controls::shapes::path_geometry> path_clip_;
    };
} // namespace maui::samples
