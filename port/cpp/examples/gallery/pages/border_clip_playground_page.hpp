#pragma once
// maui::samples::border_clip_playground_page — ports BorderClipPlayground.xaml (+ .xaml.cs)
//
// The C# page is an interactive Border-shape playground: a 100x100 Border (red stroke) clips an
// AspectFill Image (oasis.jpg) into the currently selected StrokeShape, while controls below mutate the
// shape live:
//   - a Picker (Rectangle / RoundRectangle / Ellipse; default index 1 = RoundRectangle) picks the
//     StrokeShape kind;
//   - a "Border Width" Slider (0..20, default 5) drives Border.StrokeThickness;
//   - four "Corner Radius" Sliders (TopLeft/TopRight/BottomLeft/BottomRight, each 0..60; defaults TL=60,
//     TR=0, BL=0, BR=12) feed the RoundRectangle's per-corner CornerRadius;
//   - the CornerRadius slider block is shown only while RoundRectangle is selected
//     (CornerRadiusLayout.IsVisible == SelectedIndex == 1).
// Info labels echo each slider's current value (the StringFormat readouts).
//
// This ports BorderClipPlayground.xaml.cs faithfully: UpdateBorder() builds the StrokeShape from the
// picker index (Rectangle / RoundRectangle{CornerRadius(TL,TR,BL,BR)} / Ellipse) and sets it +
// StrokeThickness on the Border; UpdateBorderShape() additionally toggles CornerRadiusLayout visibility.
//
// PORT MAPPING:
//   - Border + StrokeShape  → controls::border::set_stroke_shape with one of
//     graphics::shapes::{rectangle, round_rectangle, ellipse} (the three IShape clip geometries the port
//     ships); StrokeThickness  → set_stroke_thickness.
//   - RoundRectangle CornerRadius(TL,TR,BL,BR)  → graphics::corner_radius(tl, tr, bl, br) — the exact
//     four-arg ctor whose argument order matches the C# CornerRadius(topLeft, topRight, bottomLeft,
//     bottomRight) call in UpdateBorder.
//   - Picker SelectedIndexChanged  → picker::selected_index_changed; the index switch is read back via
//     selected_index(). The Picker is seeded with the three shape names + SelectedIndex=1 in the ctor.
//   - the Slider ValueChanged handlers  → slider::value_changed; each updates the matching readout label
//     and re-runs update_border() (the C# OnBorderWidthChanged / OnCornerRadiusChanged → UpdateBorder).
//   - CornerRadiusLayout.IsVisible  → set_visibility(visible/collapsed) on the corner-slider stack.
//   - the Image (Aspect=AspectFill, Source="oasis.jpg")  → controls::image with AspectFill +
//     image_source::from_file("oasis.jpg") (a file source the headless image path resolves synchronously;
//     the clip is the Border's StrokeShape regardless of whether the bitmap is present headless).
//
// note: the XAML wraps the controls in a <ScrollView> inside a 2-row <Grid> (the 110px preview row + a
//       "*" controls row); the port nests the preview Border + a scroll_view of the controls stack under
//       a vertical_stack_layout (the row partition is layout polish — the M2 grid row-height partition is
//       not needed to demonstrate the shape/clip mutation, which is the page's subject). The InfoStyle
//       FontSize=8 on the readouts is a markup-era Style (XAML, deferred); the readouts carry the value
//       text directly.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree (the generic mount in app_host.hpp
// attaches handlers + hosts it).

#include <cstdio>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/shapes/ellipse.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class border_clip_playground_page
    {
    public:
        border_clip_playground_page()
        {
            page_.set_title("Borders");
            controls_stack_.set_spacing(6);
            corner_stack_.set_spacing(6);

            // ---- the preview Border (red stroke, 100x100) clipping the AspectFill image ----
            border_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            border_.set_padding(maui::core::thickness(0));
            border_.set_width_request(100);
            border_.set_height_request(100);
            content_image_.set_aspect(maui::core::aspect::aspect_fill);
            content_image_.set_source(maui::controls::image_source::from_file("oasis.jpg"));
            border_.set_content(content_image_);

            // ---- Border Shape picker (Rectangle / RoundRectangle / Ellipse; default index 1) ----
            shape_heading_.set_text("Border Shape");
            shape_picker_.items().add("Rectangle");
            shape_picker_.items().add("RoundRectangle");
            shape_picker_.items().add("Ellipse");
            shape_picker_.selected_index_changed.connect([this] { on_border_shape_changed(); });

            // ---- Border Width slider (0..20, default 5) ----
            border_heading_.set_text("Border");
            width_readout_.set_text("Border Width: 5");
            width_slider_.set_minimum(0);
            width_slider_.set_maximum(20);
            width_slider_.value_changed.connect([this](double /*old_value*/, double new_value) {
                set_readout(width_readout_, "Border Width", new_value);
                update_border();
            });

            // ---- Corner Radius sliders (each 0..60; defaults TL=60, TR=0, BL=0, BR=12) ----
            corner_heading_.set_text("Corner Radius");
            init_corner(top_left_readout_, top_left_slider_, "Top Left Corner Radius", 60);
            init_corner(top_right_readout_, top_right_slider_, "Top Right Corner Radius", 0);
            init_corner(bottom_left_readout_, bottom_left_slider_, "Bottom Left Corner Radius", 0);
            init_corner(bottom_right_readout_, bottom_right_slider_, "Bottom Right Corner Radius", 12);

            corner_stack_.add(corner_heading_);
            corner_stack_.add(top_left_readout_);
            corner_stack_.add(top_left_slider_);
            corner_stack_.add(top_right_readout_);
            corner_stack_.add(top_right_slider_);
            corner_stack_.add(bottom_left_readout_);
            corner_stack_.add(bottom_left_slider_);
            corner_stack_.add(bottom_right_readout_);
            corner_stack_.add(bottom_right_slider_);

            controls_stack_.add(shape_heading_);
            controls_stack_.add(shape_picker_);
            controls_stack_.add(border_heading_);
            controls_stack_.add(width_readout_);
            controls_stack_.add(width_slider_);
            controls_stack_.add(corner_stack_);
            scroller_.set_content(controls_stack_);

            root_.set_spacing(12);
            root_.add(border_);
            root_.add(scroller_);
            page_.set_content(root_);

            // BorderClipPlayground() ctor: SelectedIndex = 1; UpdateBorder(); UpdateCornerRadius();
            shape_picker_.set_selected_index(1);
            width_slider_.set_value(5); // default Value="5" — also drives the width readout
            update_border_shape();      // toggles the corner block visible (index 1) + UpdateBorder()
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for inspection.
        [[nodiscard]] maui::controls::border& bordered()
        {
            return border_;
        }
        [[nodiscard]] maui::controls::picker& shape_picker()
        {
            return shape_picker_;
        }
        [[nodiscard]] maui::controls::slider& width_slider()
        {
            return width_slider_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& corner_stack()
        {
            return corner_stack_;
        }

    private:
        // Seed one corner slider (0..60) + its readout, and wire ValueChanged → readout + UpdateBorder.
        void init_corner(maui::controls::label& readout, maui::controls::slider& slider, const char* label,
                         double default_value)
        {
            slider.set_minimum(0);
            slider.set_maximum(60);
            slider.value_changed.connect([this, &readout, label](double /*old_value*/, double new_value) {
                set_readout(readout, label, new_value);
                update_border(); // OnCornerRadiusChanged → UpdateCornerRadius → UpdateBorder
            });
            slider.set_value(default_value);
            set_readout(readout, label, default_value);
        }

        // BorderClipPlayground.xaml.cs OnBorderShapeSelectedIndexChanged → UpdateBorderShape.
        void on_border_shape_changed()
        {
            update_border_shape();
        }

        // UpdateBorderShape(): CornerRadiusLayout.IsVisible = SelectedIndex == 1; then UpdateBorder().
        void update_border_shape()
        {
            const bool show_corners = shape_picker_.selected_index() == 1;
            corner_stack_.set_visibility(show_corners ? maui::core::visibility::visible
                                                      : maui::core::visibility::collapsed);
            update_border();
        }

        // UpdateBorder(): build the StrokeShape from the picker index and apply it + StrokeThickness.
        void update_border()
        {
            std::shared_ptr<maui::graphics::i_shape> border_shape;
            switch (shape_picker_.selected_index())
            {
                case 0:
                    border_shape = std::make_shared<maui::graphics::shapes::rectangle>();
                    break;
                case 1:
                    // RoundRectangle { CornerRadius = new CornerRadius(TL, TR, BL, BR) } — exact arg order.
                    border_shape = std::make_shared<maui::graphics::shapes::round_rectangle>(
                        maui::graphics::corner_radius(top_left_slider_.value(), top_right_slider_.value(),
                                                      bottom_left_slider_.value(), bottom_right_slider_.value()));
                    break;
                case 2:
                    border_shape = std::make_shared<maui::graphics::shapes::ellipse>();
                    break;
                default:
                    break;
            }

            border_.set_stroke_shape(std::move(border_shape));
            border_.set_stroke_thickness(width_slider_.value());
        }

        static void set_readout(maui::controls::label& readout, const char* label, double value)
        {
            char text[64];
            std::snprintf(text, sizeof(text), "%s: %.0f", label, value);
            readout.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout controls_stack_;

        // the preview Border + its clipped image
        maui::controls::border border_;
        maui::controls::image content_image_;

        // shape picker + width
        maui::controls::label shape_heading_;
        maui::controls::picker shape_picker_;
        maui::controls::label border_heading_;
        maui::controls::label width_readout_;
        maui::controls::slider width_slider_;

        // the corner-radius block (shown only for RoundRectangle)
        maui::controls::vertical_stack_layout corner_stack_;
        maui::controls::label corner_heading_;
        maui::controls::label top_left_readout_;
        maui::controls::slider top_left_slider_;
        maui::controls::label top_right_readout_;
        maui::controls::slider top_right_slider_;
        maui::controls::label bottom_left_readout_;
        maui::controls::slider bottom_left_slider_;
        maui::controls::label bottom_right_readout_;
        maui::controls::slider bottom_right_slider_;
    };
} // namespace maui::samples
