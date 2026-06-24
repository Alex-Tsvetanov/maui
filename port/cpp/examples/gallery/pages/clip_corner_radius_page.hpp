#pragma once
// maui::samples::clip_corner_radius_page — ports ClipCornerRadiusGallery.xaml (+ .xaml.cs)
//
// The C# page (Pages/Controls/ShapesGalleries/ClipCornerRadiusGallery.xaml) is a StackLayout
// (Padding=12) that demonstrates DRIVING a RoundRectangleGeometry's per-corner CornerRadius from four
// sliders, live, while it clips an image:
//
//   - a Grid container (Style ImageContainerStyle: LightGray, 200x200, HorizontalOptions=Start) holding
//     a single <Image Source="oasis.jpg" AspectFill 200x200> whose Image.Clip is a
//     <RoundRectangleGeometry x:Name="RoundRectangleGeometry" Rect="0,0,150,150"/> (CornerRadius
//     defaults to 0 — square corners initially),
//   - four labeled Sliders (x:Names TopLeftCorner / TopRightCorner / BottomLeftCorner /
//     BottomRightCorner), each Minimum=0 Maximum=60 (the <Style TargetType="Slider">), every one wired
//     to ValueChanged="OnCornerChanged".
//
// The code-behind (ClipCornerRadiusGallery.xaml.cs) OnCornerChanged rebuilds the clip's CornerRadius
// from all four slider values, in CornerRadius(topLeft, topRight, bottomLeft, bottomRight) order:
//
//   RoundRectangleGeometry.CornerRadius = new CornerRadius(
//       TopLeftCorner.Value, TopRightCorner.Value, BottomLeftCorner.Value, BottomRightCorner.Value);
//
// PORT MAPPING:
//   - RoundRectangleGeometry(Rect="0,0,150,150")  -> shapes::round_rectangle_geometry, set_rect(rect{0,0,150,150})
//     (round_rectangle_geometry : geometry_group : geometry : i_shape — so view::set_clip takes it 1:1).
//   - Slider Minimum=0 Maximum=60                  -> slider::set_minimum(0) / set_maximum(60).
//   - Slider.ValueChanged="OnCornerChanged"        -> slider::value_changed.connect([this](double, double){ … }).
//     The (old, new) payload is ignored; the handler re-reads all four sliders' current ::value() and
//     rebuilds set_corner_radius, exactly like the C# handler reads the four .Value properties.
//   - CornerRadius(TL, TR, BL, BR)                 -> graphics::corner_radius{tl, tr, bl, br} (same 4-arg order).
//   - MinimumTrackColor=LightGray, MaximumTrackColor=Gray (the Slider style) -> set_minimum/maximum_track_color.
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree (the generic mount in app_host.hpp
// attaches every owned view's handler and hosts the tree). This is a
// VISUAL + INTERACTIVE clip demo: moving a slider reshapes the corner that clips the image (observable
// here via the geometry's corner_radius being driven deterministically; a backend that renders draws it).
//
// note: this page's root is a bare StackLayout (NO ScrollView, faithfully to the XAML). The
//       Slider style's HorizontalOptions="FillAndExpand" is layout polish the headless slider surface
//       does not expose, so it is left best-effort; the FontSize="Medium"/"Small" label styling is
//       likewise best-effort (the caption text is what is reproduced). Source="oasis.jpg" is a
//       best-effort file_image_source — the demonstrated feature is the live corner-radius clip.

#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/shapes/round_rectangle_geometry.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class clip_corner_radius_page
    {
    public:
        clip_corner_radius_page()
        {
            page_.set_title("Clip CornerRadius Gallery");
            stack_.set_padding(maui::core::thickness{12}); // StackLayout Padding="12"
            stack_.set_spacing(6);

            // The named clip: RoundRectangleGeometry Rect="0,0,150,150", CornerRadius default 0 (square).
            round_rect_clip_ = std::make_shared<maui::controls::shapes::round_rectangle_geometry>();
            round_rect_clip_->set_rect(maui::graphics::rect{0, 0, 150, 150});

            // Caption above the container.
            container_label_.set_text("Clipped Image using RoundRectangleGeometry");
            stack_.add(container_label_);

            // Grid container (ImageContainerStyle: LightGray, 200x200, Start) holding the clipped image.
            container_.set_height_request(200);
            container_.set_width_request(200);
            container_.set_background(std::make_shared<maui::graphics::solid_paint>(
                maui::graphics::color::from_rgb(211, 211, 211))); // LightGray = #D3D3D3
            // Image Source="oasis.jpg" AspectFill 200x200, clipped by the named round-rectangle geometry.
            image_.set_source(std::make_shared<maui::controls::file_image_source>("oasis.jpg"));
            image_.set_aspect(maui::core::aspect::aspect_fill);
            image_.set_width_request(200);
            image_.set_height_request(200);
            image_.set_clip(round_rect_clip_);
            container_.add(image_);
            stack_.add(container_);

            // The four labeled corner sliders, in the XAML order: TL, TR, BL, BR. Each Minimum=0
            // Maximum=60, MinimumTrackColor=LightGray, MaximumTrackColor=Gray (the Slider style), every
            // one wired to the shared OnCornerChanged.
            add_corner_slider(top_left_label_, "Top Left Corner", top_left_corner_);
            add_corner_slider(top_right_label_, "Top Right Corner", top_right_corner_);
            add_corner_slider(bottom_left_label_, "Bottom Left Corner", bottom_left_corner_);
            add_corner_slider(bottom_right_label_, "Bottom Right Corner", bottom_right_corner_);

            page_.set_content(stack_);
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
        [[nodiscard]] maui::controls::grid& container()
        {
            return container_;
        }
        [[nodiscard]] maui::controls::image& image()
        {
            return image_;
        }
        [[nodiscard]] maui::controls::slider& top_left_corner()
        {
            return top_left_corner_;
        }
        [[nodiscard]] maui::controls::slider& top_right_corner()
        {
            return top_right_corner_;
        }
        [[nodiscard]] maui::controls::slider& bottom_left_corner()
        {
            return bottom_left_corner_;
        }
        [[nodiscard]] maui::controls::slider& bottom_right_corner()
        {
            return bottom_right_corner_;
        }
        // The named clip, exposed for inspection of the driven CornerRadius.
        [[nodiscard]] maui::controls::shapes::round_rectangle_geometry& round_rectangle_geometry()
        {
            return *round_rect_clip_;
        }

    private:
        // Add one "<Label Text=…> over a corner Slider" pair: Minimum=0 Maximum=60, track colors per the
        // Slider style, ValueChanged="OnCornerChanged".
        void add_corner_slider(maui::controls::label& text, const char* caption, maui::controls::slider& corner)
        {
            text.set_text(caption);
            stack_.add(text);

            corner.set_minimum(0);
            corner.set_maximum(60);
            corner.set_minimum_track_color(maui::graphics::color::from_rgb(211, 211, 211)); // LightGray
            corner.set_maximum_track_color(maui::graphics::color::from_rgb(128, 128, 128)); // Gray
            // ValueChanged="OnCornerChanged": the (old, new) payload is unused; the handler re-reads all
            // four sliders and rebuilds the clip's CornerRadius, exactly like the C# handler.
            corner.value_changed.connect([this](double, double) { on_corner_changed(); });
            stack_.add(corner);
        }

        // ClipCornerRadiusGallery.xaml.cs OnCornerChanged: rebuild the named geometry's CornerRadius from
        // the four sliders, in CornerRadius(topLeft, topRight, bottomLeft, bottomRight) order.
        void on_corner_changed()
        {
            round_rect_clip_->set_corner_radius(
                maui::graphics::corner_radius{top_left_corner_.value(), top_right_corner_.value(),
                                              bottom_left_corner_.value(), bottom_right_corner_.value()});
            // Re-set the clip so the image re-evaluates its clip shape (the clip points at the same
            // geometry instance; this makes the property write observable to the view's handler).
            image_.set_clip(round_rect_clip_);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label container_label_;
        maui::controls::grid container_; // ImageContainerStyle Grid (LightGray, 200x200)
        maui::controls::image image_;

        maui::controls::label top_left_label_;
        maui::controls::slider top_left_corner_;
        maui::controls::label top_right_label_;
        maui::controls::slider top_right_corner_;
        maui::controls::label bottom_left_label_;
        maui::controls::slider bottom_left_corner_;
        maui::controls::label bottom_right_label_;
        maui::controls::slider bottom_right_corner_;

        // The named RoundRectangleGeometry the four sliders drive.
        std::shared_ptr<maui::controls::shapes::round_rectangle_geometry> round_rect_clip_;
    };
} // namespace maui::samples
