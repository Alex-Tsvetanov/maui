#pragma once
// maui::samples::box_view_page — ports BoxViewPage.xaml (+ BoxViewPage.xaml.cs).
//
// The C# BoxViewPage is a ScrollView over a VerticalStackLayout of headline-labelled box_view
// variants, each a 160x160 centered block demonstrating one facet of the control:
//   Default (BackgroundColor), Using Color, Background (a LinearGradientBrush), Using CornerRadius
//   (uniform 10), Using Complex CornerRadius (10,0,5,20), Using Opacity (0.5), Clip (an
//   EllipseGeometry), and Shadow (radius 6, offset 6,6, red brush).
//
// This is a code-first port following the value_controls_page / shapes_page pattern: the page OWNS
// its whole element tree as members, exposes page() and attach_handlers(maui_app). It is
// headless-safe — only cross-platform maui:: API. The original .xaml.cs has no logic beyond
// InitializeComponent(), so there are no events to wire; the demonstration is purely the set of
// statically-configured box_view variants.
//
// Fidelity notes:
//   - The C# "Default" box uses BackgroundColor=CornflowerBlue (VisualElement.BackgroundColor); the
//     port sets a solid_paint background to mirror it. "Using Color" uses box_view::set_color (the
//     BoxView.Color shape fill). Both are faithful to the two distinct C# code paths.
//   - The gradient "Background" variant is wired with a real linear_gradient_paint (Yellow@0.1 ->
//     Green@1.0, end_point (1,0)), matching the XAML LinearGradientBrush exactly.
//   - Complex CornerRadius maps to the 4-arg corner_radius(top_left, top_right, bottom_left,
//     bottom_right) — note: MAUI's CornerRadius 4-arg order is (TL, TR, BL, BR), matching the port.
//   - Opacity, CornerRadius (uniform), and Shadow are wired faithfully.
//   - note: the C# "Clip" variant uses an EllipseGeometry (a Geometry, not a Shape). The port's
//     view::set_clip seam takes a graphics::i_shape; there is no Geometry-as-clip primitive in the
//     headless surface yet, so the clip box is rendered as a plain pink block (best-effort) rather
//     than fabricating a clip. Everything else is a 1:1 port.

#include <memory>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class box_view_page
    {
    public:
        box_view_page()
        {
            page_.set_title("BoxView");
            stack_.set_padding(maui::core::thickness(12));
            stack_.set_spacing(6);

            // ---- Default: BackgroundColor=CornflowerBlue (the VisualElement.BackgroundColor path) ----
            default_label_.set_text("Default");
            default_box_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::cornflower_blue));
            size_box(default_box_);

            // ---- Using Color: BoxView.Color=Purple (the shape-fill path) ----
            color_label_.set_text("Using Color");
            color_box_.set_color(maui::graphics::colors::purple);
            size_box(color_box_);

            // ---- Background: a LinearGradientBrush (Yellow@0.1 -> Green@1.0, end_point (1,0)) ----
            gradient_label_.set_text("Background");
            auto gradient = std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{
                    maui::graphics::gradient_stop(0.1F, maui::graphics::colors::yellow),
                    maui::graphics::gradient_stop(1.0F, maui::graphics::colors::green)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0));
            gradient_box_.set_background(gradient);
            size_box(gradient_box_);

            // ---- Using CornerRadius: Color=LightGreen, uniform radius 10 ----
            corner_label_.set_text("Using CornerRadius");
            corner_box_.set_color(maui::graphics::colors::light_green);
            corner_box_.set_corner_radius(maui::graphics::corner_radius(10));
            size_box(corner_box_);

            // ---- Using Complex CornerRadius: Color=Orange, radii (10, 0, 5, 20) = TL,TR,BL,BR ----
            complex_corner_label_.set_text("Using Complex CornerRadius");
            complex_corner_box_.set_color(maui::graphics::colors::orange);
            complex_corner_box_.set_corner_radius(maui::graphics::corner_radius(10, 0, 5, 20));
            size_box(complex_corner_box_);

            // ---- Using Opacity: Color=Orange, Opacity=0.5 ----
            opacity_label_.set_text("Using Opacity");
            opacity_box_.set_color(maui::graphics::colors::orange);
            opacity_box_.set_opacity(0.5);
            size_box(opacity_box_);

            // ---- Clip: Color=Pink (the EllipseGeometry clip is a Geometry, not an i_shape; note above) ----
            clip_label_.set_text("Clip");
            clip_box_.set_color(maui::graphics::colors::pink);
            size_box(clip_box_);
            // note: EllipseGeometry clip deferred — the view::set_clip seam takes graphics::i_shape and
            // there is no Geometry-as-clip primitive in the headless surface; rendered as a plain block.

            // ---- Shadow: Color=Pink, Shadow{ radius 6, offset (6,6), red brush } ----
            shadow_label_.set_text("Shadow");
            shadow_box_.set_color(maui::graphics::colors::pink);
            auto box_shadow = std::make_shared<maui::core::shadow>();
            box_shadow->set_radius(6);
            box_shadow->set_offset(maui::graphics::point(6, 6));
            box_shadow->set_color(maui::graphics::colors::red);
            shadow_box_.set_shadow(box_shadow);
            size_box(shadow_box_);

            stack_.add(default_label_);
            stack_.add(default_box_);
            stack_.add(color_label_);
            stack_.add(color_box_);
            stack_.add(gradient_label_);
            stack_.add(gradient_box_);
            stack_.add(corner_label_);
            stack_.add(corner_box_);
            stack_.add(complex_corner_label_);
            stack_.add(complex_corner_box_);
            stack_.add(opacity_label_);
            stack_.add(opacity_box_);
            stack_.add(clip_label_);
            stack_.add(clip_box_);
            stack_.add(shadow_label_);
            stack_.add(shadow_box_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves -> stack -> scroll -> page), then re-host
        // the tree built in the ctor (gallery_attach.hpp). EXCLUDES the non-view items (paints, shadow).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) {
                try
                {
                    app.attach_handler(view);
                }
                catch (const std::exception& error)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", name, error.what());
                }
            };

            one(default_label_, "default_label_");
            one(default_box_, "default_box_");
            one(color_label_, "color_label_");
            one(color_box_, "color_box_");
            one(gradient_label_, "gradient_label_");
            one(gradient_box_, "gradient_box_");
            one(corner_label_, "corner_label_");
            one(corner_box_, "corner_box_");
            one(complex_corner_label_, "complex_corner_label_");
            one(complex_corner_box_, "complex_corner_box_");
            one(opacity_label_, "opacity_label_");
            one(opacity_box_, "opacity_box_");
            one(clip_label_, "clip_label_");
            one(clip_box_, "clip_box_");
            one(shadow_label_, "shadow_label_");
            one(shadow_box_, "shadow_box_");
            one(stack_, "stack_");
            one(scroll_, "scroll_");
            one(page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroll_);
            gallery_rehost_content(page_);
        }

    private:
        // Each XAML box is 160x160 (the shared block geometry).
        // note: the XAML Horizontal/VerticalOptions="Center" have no settable port equivalent —
        // view::horizontal_layout_alignment / vertical_layout_alignment are fixed `fill` overrides on
        // this surface, so only the explicit 160x160 size request is wired (faithful best-effort).
        static void size_box(maui::controls::box_view& box)
        {
            box.set_width_request(160);
            box.set_height_request(160);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_label_;
        maui::controls::box_view default_box_;
        maui::controls::label color_label_;
        maui::controls::box_view color_box_;
        maui::controls::label gradient_label_;
        maui::controls::box_view gradient_box_;
        maui::controls::label corner_label_;
        maui::controls::box_view corner_box_;
        maui::controls::label complex_corner_label_;
        maui::controls::box_view complex_corner_box_;
        maui::controls::label opacity_label_;
        maui::controls::box_view opacity_box_;
        maui::controls::label clip_label_;
        maui::controls::box_view clip_box_;
        maui::controls::label shadow_label_;
        maui::controls::box_view shadow_box_;
    };
} // namespace maui::samples
