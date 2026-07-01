#pragma once
// maui::samples::clip_page — ports ClipPage.xaml
//
// The C# page (Pages/Core/ClipPage.xaml; its .xaml.cs is an empty InitializeComponent) is a ScrollView
// over a StackLayout that shows the SAME dotnet_bot.png image five times, each successive copy carrying a
// different geometry on its Image.Clip (the VisualElement.Clip / IView.Clip surface):
//   - the bare image (no clip),
//   - a RectangleGeometry  Rect="0,15,150,150",
//   - an EllipseGeometry   Center="100,100" RadiusX=100 RadiusY=100,
//   - a GeometryGroup      (FillRule=EvenOdd) of four overlapping ellipses, and
//   - a PathGeometry       from the markup "M8 148 L156 148 L132 12 Z" (a triangle).
// Each is captioned by a "Headline" label. (The XAML <Style TargetType="Image"> sets a LightGray
// background, AspectFill, 200x200, HorizontalOptions=Start — applied below per image.)
//
// PORT MAPPING (every clip geometry is a maui::controls::shapes::geometry, and geometry : i_shape — see
// controls/shapes/geometry.hpp — so each ports 1:1 to view::set_clip(shared_ptr<i_shape>)):
//   - RectangleGeometry(Rect)            -> shapes::rectangle_geometry(rect{0,15,150,150})
//   - EllipseGeometry(Center,Rx,Ry)      -> shapes::ellipse_geometry(point{100,100}, 100, 100)
//   - GeometryGroup{ EllipseGeometry… }  -> shapes::geometry_group (FillRule even_odd, four ellipse children)
//   - PathGeometry(Figures="…")          -> shapes::path_geometry, figures parsed from the markup
//
// The XAML has no interaction (no code-behind), but the gallery convention is an observable readout, so
// this port ADDS a "Toggle clip" button + a status label: the button clears every image's clip (and
// re-applies it on the next press), and the status echoes "Clipped" / "Cleared" — the same five clip
// geometries, now toggled so the clip surface is exercised programmatically (the shapes render clipped
// when on; headless has no pixels but the clip property and re-host are driven deterministically).
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree (the generic mount in app_host.hpp
// attaches every owned view's handler and hosts the tree).
//
// note: the C# Source="dotnet_bot.png" is a file image source; the headless gallery loads no pixels, so
//       each image carries a best-effort file_image_source("dotnet_bot.png") — the demonstrated feature
//       is the Clip geometry, not the bitmap. The Style's LightGray background + AspectFill + 200x200 are
//       applied so the clipped region is observable in a backend that does render.

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "maui/controls/button.hpp"
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
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class clip_page
    {
    public:
        clip_page()
        {
            page_.set_title("Clip");
            stack_.set_spacing(6);

            // ---- the four clip geometries, built once and kept so the toggle can re-apply them ----
            // RectangleGeometry Rect="0, 15, 150, 150".
            rect_clip_ =
                std::make_shared<maui::controls::shapes::rectangle_geometry>(maui::graphics::rect{0, 15, 150, 150});

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

            // ---- the five captioned images (the first bare, the rest each carrying one clip) ----
            caption(image_label_, "Image");
            style_image(image_);
            stack_.add(image_);

            caption(rect_label_, "Clipped Image using RectangleGeometry");
            style_image(rect_image_);
            rect_image_.set_clip(rect_clip_);
            stack_.add(rect_image_);

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

            // ---- the gallery readout + toggle (the observable extension; see header) ----
            status_.set_text("Clipped");
            stack_.add(status_);

            // A real button so the click is observable; bottom-up attach handles it.
            toggle_.set_text("Toggle clip on/off");
            toggle_.clicked.connect([this]() { on_toggle(); });
            stack_.add(toggle_);

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
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::button& toggle()
        {
            return toggle_;
        }
        [[nodiscard]] maui::controls::image& rect_image()
        {
            return rect_image_;
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
        // Toggle every clipped image's geometry off (set null) or back on (re-apply the stored geometry),
        // echoing the new state into the status label. The first (bare) image stays unclipped throughout.
        void on_toggle()
        {
            clipped_ = !clipped_;
            rect_image_.set_clip(clipped_ ? rect_clip_ : nullptr);
            ellipse_image_.set_clip(clipped_ ? ellipse_clip_ : nullptr);
            group_image_.set_clip(clipped_ ? group_clip_ : nullptr);
            path_image_.set_clip(clipped_ ? path_clip_ : nullptr);
            status_.set_text(clipped_ ? "Clipped" : "Cleared");
        }

        // The XAML <Style TargetType="Image">: LightGray background, AspectFill, 200x200, Start alignment
        // (the source bitmap is best-effort; see header).
        static void style_image(maui::controls::image& picture)
        {
            picture.set_source(std::make_shared<maui::controls::file_image_source>("dotnet_bot.png"));
            picture.set_aspect(maui::core::aspect::aspect_fill);
            picture.set_width_request(200);
            picture.set_height_request(200);
            // HorizontalOptions="Start" (the Style setter) — left-align the 200-wide box so it (and its gray
            // background) frames the left edge, matching MAUI. Without this the default fill/center placement
            // centers the box, which diverged from the reference.
            picture.set_horizontal_layout_alignment(maui::core::layout_alignment::start);
            picture.set_background(std::make_shared<maui::graphics::solid_paint>(
                maui::graphics::color::from_rgb(211, 211, 211))); // LightGray = #D3D3D3
        }

        // One "Headline"-style caption label above an image.
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        bool clipped_ = true;

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label image_label_;
        maui::controls::image image_;
        maui::controls::label rect_label_;
        maui::controls::image rect_image_;
        maui::controls::label ellipse_label_;
        maui::controls::image ellipse_image_;
        maui::controls::label group_label_;
        maui::controls::image group_image_;
        maui::controls::label path_label_;
        maui::controls::image path_image_;

        maui::controls::label status_;
        maui::controls::button toggle_;

        // The four clip geometries (kept so the toggle can re-apply them after clearing).
        std::shared_ptr<maui::controls::shapes::rectangle_geometry> rect_clip_;
        std::shared_ptr<maui::controls::shapes::ellipse_geometry> ellipse_clip_;
        std::shared_ptr<maui::controls::shapes::geometry_group> group_clip_;
        std::shared_ptr<maui::controls::shapes::path_geometry> path_clip_;
    };
} // namespace maui::samples
