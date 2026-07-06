#pragma once
// maui::samples::transform_playground_page — ports TransformPlaygroundGallery.xaml
//
// A code-first port of the MAUI Shapes sub-gallery
// Pages/Controls/ShapesGalleries/TransformPlaygroundGallery.xaml: a 50x50 Path rectangle (red fill,
// blue stroke 4) sits in a 200x200 light-grey panel; below it a ScrollView of Sliders drives the
// Path's RenderTransform — a TransformGroup that folds, in C#'s authored order:
//   RotateTransform(Angle; CenterX, CenterY)
//   ScaleTransform (ScaleX, ScaleY; CenterX, CenterY)
//   SkewTransform  (AngleX, AngleY; CenterX, CenterY)
//   TranslateTransform(X, Y).
// One shared CenterX / CenterY pair feeds the rotate, scale and skew centers (the XAML binds all
// three to the SAME two SliderCenterX / SliderCenterY sources). A per-row Label echoes the live
// slider value (the XAML's `{Binding Value, StringFormat=…}`).
//
// Demonstrated (all headless-safe maui:: shape transforms):
//   path::set_render_transform over a transform_group of rotate_transform / scale_transform /
//   skew_transform / translate_transform, re-pushed via path::invalidate_render_transform() on each
//   slider value_changed — the live RenderTransform seam the C# binding engine drives.
//
// The page OWNS its whole element tree (the transformations_page / path_gallery_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page()
// in a window; the headless/apple/ios test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# code-behind is just InitializeComponent() — no logic to port; all behavior is the
//         RenderTransform bindings, reproduced here as explicit slider value_changed handlers (no XAML
//         binding engine, layer 6).
//   note: the C# page lays the panel + the slider ScrollView out in a 2-row Grid (RowDefinitions
//         Auto / *). The port composes the panel and the ScrollView into the page's own
//         vertical_stack_layout instead — the same top-to-bottom reading order — because a
//         row-defined Grid host is a layout-wave concern; the visible content and its order are
//         unchanged (best-effort).
//   note: the C# transform target is a <Path> whose Data is a <RectangleGeometry Rect="0,0,50,50">.
//         The port builds the identical geometry programmatically (rectangle_geometry over rect
//         {0,0,50,50}) and feeds it to path::set_data — object-element geometry authoring is the XAML
//         wave's job, and the resolved geometry is the same.
//   note: the C# panel BackgroundColor="#e5e5e5", the Path Fill="Red" / Stroke="Blue" /
//         StrokeThickness=4, and the Slider track/thumb colors are reconstructed via the documented
//         brush→paint bridge (solid_paint over a color) and the slider color setters.
//   note: the shared CenterX / CenterY sliders drive THREE transforms' centers at once (rotate, scale,
//         skew), exactly as the XAML binds all three centers to the same two sources — each slider's
//         value_changed updates every dependent transform's center, then re-pushes the group.
//   note: the C# default Slider Value is 0 (Min 0) except ScaleX / ScaleY (Value=1, range 0.5..2);
//         the port seeds the scale sliders + transforms to 1 to match, and the scale slider Minimum to
//         0.5 (so the thumb starts mid-track as in the XAML).

#include <cstdio>
#include <memory>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/rectangle_geometry.hpp"
#include "maui/controls/shapes/rotate_transform.hpp"
#include "maui/controls/shapes/scale_transform.hpp"
#include "maui/controls/shapes/skew_transform.hpp"
#include "maui/controls/shapes/transform_group.hpp"
#include "maui/controls/shapes/translate_transform.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class transform_playground_page
    {
    public:
        transform_playground_page()
        {
            page_.set_title("Transform Playground");
            stack_.set_spacing(0);

            build_target();
            build_controls();

            // ---- assemble: the 200x200 panel (target) first, then the ScrollView of slider rows ----
            panel_.set_background(solid(maui::graphics::color::from_argb("#e5e5e5"))); // C# BackgroundColor
            panel_.set_width_request(200);
            panel_.set_height_request(200);
            // C# TransformPlaygroundPage: the 200x200 panel sets NO HorizontalOptions, so it keeps the
            // default Fill+explicit-width behaviour (view::align_horizontal centers it). Do NOT force Start
            // here — maui-compare renders this panel centered. (Reverts an over-application of fix #4.)
            panel_.add(path_);

            controls_stack_.set_spacing(0);
            controls_stack_.set_padding(maui::core::thickness{12}); // C# ScrollView Padding="12"

            controls_stack_.add(rotate_header_);
            controls_stack_.add(rotation_readout_);
            controls_stack_.add(rotation_slider_);
            controls_stack_.add(center_x_readout_);
            controls_stack_.add(center_x_slider_);
            controls_stack_.add(center_y_readout_);
            controls_stack_.add(center_y_slider_);

            controls_stack_.add(scale_header_);
            controls_stack_.add(scale_x_readout_);
            controls_stack_.add(scale_x_slider_);
            controls_stack_.add(scale_y_readout_);
            controls_stack_.add(scale_y_slider_);

            controls_stack_.add(skew_header_);
            controls_stack_.add(skew_x_readout_);
            controls_stack_.add(skew_x_slider_);
            controls_stack_.add(skew_y_readout_);
            controls_stack_.add(skew_y_slider_);

            controls_stack_.add(translate_header_);
            controls_stack_.add(translate_x_readout_);
            controls_stack_.add(translate_x_slider_);
            controls_stack_.add(translate_y_readout_);
            controls_stack_.add(translate_y_slider_);

            scroll_.set_content(controls_stack_);

            stack_.add(panel_);
            stack_.add(scroll_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::shapes::path& target()
        {
            return path_;
        }
        [[nodiscard]] maui::controls::slider& rotation_slider()
        {
            return rotation_slider_;
        }
        [[nodiscard]] maui::controls::slider& scale_x_slider()
        {
            return scale_x_slider_;
        }
        [[nodiscard]] maui::controls::slider& center_x_slider()
        {
            return center_x_slider_;
        }
        [[nodiscard]] maui::controls::shapes::transform_group& transform()
        {
            return *group_;
        }

    private:
        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        // Build the transform target: a 200x200 Path whose Data is RectangleGeometry(0,0,50,50), red
        // fill, blue stroke 4, carrying the live TransformGroup as its RenderTransform. The original C#
        // TransformPlaygroundGallery.xaml places the 50x50 square at the LITERAL (0,0) origin of its
        // 200x200 canvas (top-left) — matching this builder — with the panel's own PathContainerStyle
        // set to HorizontalOptions="Start" VerticalOptions="Start" (see the panel_ setup above). The
        // canonical shared transform_playground.xaml's degraded Rectangle-in-VerticalStackLayout twin
        // was missing that Start/Start declaration (an authoring omission — fixed separately in the
        // shared XAML, not here).
        void build_target()
        {
            path_.set_data(
                std::make_shared<maui::controls::shapes::rectangle_geometry>(maui::graphics::rect{0, 0, 50, 50}));
            path_.set_fill(solid(maui::graphics::colors::red));
            path_.set_stroke(solid(maui::graphics::colors::blue));
            path_.set_stroke_thickness(4);
            path_.set_width_request(200);
            path_.set_height_request(200);

            // The C#-ordered TransformGroup: rotate, scale, skew, translate (folded left-to-right).
            group_ = std::make_shared<maui::controls::shapes::transform_group>();
            group_->children().push_back(rotate_);
            group_->children().push_back(scale_);
            group_->children().push_back(skew_);
            group_->children().push_back(translate_);
            scale_->set_scale_x(1); // C# ScaleX/ScaleY default Value=1
            scale_->set_scale_y(1);
            path_.set_render_transform(group_);
        }

        // Wire each slider to its transform scalar, with a readout label echoing the value, then seed the
        // readouts. Re-push the group after every change (the live RenderTransform seam).
        void build_controls()
        {
            // ---- ROTATE: Rotation (0..200), CenterX (0..100), CenterY (0..100) ----
            rotate_header_.set_text("RotateTransform");
            rotation_slider_.set_minimum(0);
            rotation_slider_.set_maximum(200);
            rotation_slider_.value_changed.connect([this](double, double v) {
                rotate_->set_angle(v);
                update_readout(rotation_readout_, "Rotation", v, 0);
                repush();
            });
            update_readout(rotation_readout_, "Rotation", rotate_->angle(), 0);

            center_x_slider_.set_minimum(0);
            center_x_slider_.set_maximum(100);
            center_x_slider_.value_changed.connect([this](double, double v) {
                // the shared CenterX feeds rotate + scale + skew (the XAML binds all three to it)
                rotate_->set_center_x(v);
                scale_->set_center_x(v);
                skew_->set_center_x(v);
                update_readout(center_x_readout_, "CenterX", v, 0);
                repush();
            });
            update_readout(center_x_readout_, "CenterX", rotate_->center_x(), 0);

            center_y_slider_.set_minimum(0);
            center_y_slider_.set_maximum(100);
            center_y_slider_.value_changed.connect([this](double, double v) {
                rotate_->set_center_y(v);
                scale_->set_center_y(v);
                skew_->set_center_y(v);
                update_readout(center_y_readout_, "CenterY", v, 0);
                repush();
            });
            update_readout(center_y_readout_, "CenterY", rotate_->center_y(), 0);

            // ---- SCALE: ScaleX / ScaleY (0.5..2, default 1) ----
            scale_header_.set_text("ScaleTransform");
            scale_x_slider_.set_minimum(0.5);
            scale_x_slider_.set_maximum(2);
            scale_x_slider_.set_value(1);
            scale_x_slider_.value_changed.connect([this](double, double v) {
                scale_->set_scale_x(v);
                update_readout(scale_x_readout_, "ScaleX", v, 2);
                repush();
            });
            update_readout(scale_x_readout_, "ScaleX", scale_->scale_x(), 2);

            scale_y_slider_.set_minimum(0.5);
            scale_y_slider_.set_maximum(2);
            scale_y_slider_.set_value(1);
            scale_y_slider_.value_changed.connect([this](double, double v) {
                scale_->set_scale_y(v);
                update_readout(scale_y_readout_, "ScaleY", v, 2);
                repush();
            });
            update_readout(scale_y_readout_, "ScaleY", scale_->scale_y(), 2);

            // ---- SKEW: SkewX / SkewY (0..100) ----
            skew_header_.set_text("SkewTransform");
            skew_x_slider_.set_minimum(0);
            skew_x_slider_.set_maximum(100);
            skew_x_slider_.value_changed.connect([this](double, double v) {
                skew_->set_angle_x(v);
                update_readout(skew_x_readout_, "SkewX", v, 0);
                repush();
            });
            update_readout(skew_x_readout_, "SkewX", skew_->angle_x(), 0);

            skew_y_slider_.set_minimum(0);
            skew_y_slider_.set_maximum(100);
            skew_y_slider_.value_changed.connect([this](double, double v) {
                skew_->set_angle_y(v);
                update_readout(skew_y_readout_, "SkewY", v, 0);
                repush();
            });
            update_readout(skew_y_readout_, "SkewY", skew_->angle_y(), 0);

            // ---- TRANSLATE: X / Y (0..200) ----
            translate_header_.set_text("TranslateTransform");
            translate_x_slider_.set_minimum(0);
            translate_x_slider_.set_maximum(200);
            translate_x_slider_.value_changed.connect([this](double, double v) {
                translate_->set_x(v);
                update_readout(translate_x_readout_, "X", v, 0);
                repush();
            });
            update_readout(translate_x_readout_, "X", translate_->x(), 0);

            translate_y_slider_.set_minimum(0);
            translate_y_slider_.set_maximum(200);
            translate_y_slider_.value_changed.connect([this](double, double v) {
                translate_->set_y(v);
                update_readout(translate_y_readout_, "Y", v, 0);
                repush();
            });
            update_readout(translate_y_readout_, "Y", translate_->y(), 0);
        }

        // Re-run the "render_transform" mapper after an in-place transform-scalar mutation (the group is
        // owned by the path; mutating a child does not auto-invalidate — path.hpp's documented seam).
        void repush()
        {
            path_.invalidate_render_transform();
        }

        // Set `label` to "<name>: <value with `decimals` digits>" (the XAML StringFormat rows).
        static void update_readout(maui::controls::label& label, const char* name, double value, int decimals)
        {
            char text[64];
            (void)std::snprintf(text, sizeof(text), "%s: %.*f", name, decimals, value);
            label.set_text(text);
        }

        // ---- hosts ----
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;          // the page's top-level column (Grid stand-in)
        maui::controls::vertical_stack_layout panel_;          // the 200x200 light-grey transform panel
        maui::controls::scroll_view scroll_;                   // scrolls the slider rows
        maui::controls::vertical_stack_layout controls_stack_; // the column of slider rows

        // ---- the transform target ----
        maui::controls::shapes::path path_;
        std::shared_ptr<maui::controls::shapes::transform_group> group_;
        std::shared_ptr<maui::controls::shapes::rotate_transform> rotate_ =
            std::make_shared<maui::controls::shapes::rotate_transform>();
        std::shared_ptr<maui::controls::shapes::scale_transform> scale_ =
            std::make_shared<maui::controls::shapes::scale_transform>();
        std::shared_ptr<maui::controls::shapes::skew_transform> skew_ =
            std::make_shared<maui::controls::shapes::skew_transform>();
        std::shared_ptr<maui::controls::shapes::translate_transform> translate_ =
            std::make_shared<maui::controls::shapes::translate_transform>();

        // ---- ROTATE rows ----
        maui::controls::label rotate_header_;
        maui::controls::label rotation_readout_;
        maui::controls::slider rotation_slider_;
        maui::controls::label center_x_readout_;
        maui::controls::slider center_x_slider_;
        maui::controls::label center_y_readout_;
        maui::controls::slider center_y_slider_;

        // ---- SCALE rows ----
        maui::controls::label scale_header_;
        maui::controls::label scale_x_readout_;
        maui::controls::slider scale_x_slider_;
        maui::controls::label scale_y_readout_;
        maui::controls::slider scale_y_slider_;

        // ---- SKEW rows ----
        maui::controls::label skew_header_;
        maui::controls::label skew_x_readout_;
        maui::controls::slider skew_x_slider_;
        maui::controls::label skew_y_readout_;
        maui::controls::slider skew_y_slider_;

        // ---- TRANSLATE rows ----
        maui::controls::label translate_header_;
        maui::controls::label translate_x_readout_;
        maui::controls::slider translate_x_slider_;
        maui::controls::label translate_y_readout_;
        maui::controls::slider translate_y_slider_;
    };
} // namespace maui::samples
