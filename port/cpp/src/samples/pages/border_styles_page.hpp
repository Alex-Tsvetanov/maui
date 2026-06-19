#pragma once
// maui::samples::border_styles_page — ports BorderStyles.xaml (+ BorderStyles.xaml.cs)
//
// A self-contained, code-first demo of styling a Border and mutating its StrokeShape at runtime. It
// mirrors the C# gallery page (Pages/Core/BorderGalleries/BorderStyles.xaml): a margined, 12-spaced
// StackLayout of "Headline"-captioned Border sections —
//   - "RoundRectangle 10,10,10,10": a Border carrying the C# <Style x:Key="BorderStyle"> — a
//     RoundRectangle(10) stroke shape, red stroke, thickness 1, height 55, padding 10, transparent bg;
//   - "StrokeShape binded to Parent": the C# <Style x:Key="StrokeShapeBindingParentBorderStyle"> whose
//     RoundRectangle CornerRadius binds to the parent Border's Width — same red/1/55/10/transparent
//     surface, the corner radius following the resolved width (note: the live width-binding is a deferred
//     XAML/RelativeSource facility — see note);
//   - "Update StrokeShape": the x:Name="UpdateStrokeShapeBorder" — 400x200, yellow background, red stroke
//     thickness 2, a RoundRectangle(10) stroke shape, hosting the x:Name="UpdateStrokeShapeInfo" readout
//     label; and
//   - two Buttons ("Increase/Decrease CornerRadius") driving OnIncreaseCornerRadius/OnDecreaseCornerRadius.
//
// The C# code-behind's ChangeCornerRadius(delta) reads the border's RoundRectangle stroke shape, clamps
// radius = max(0, current + delta), writes the new CornerRadius back, and sets the info label to
// "Border.StrokeShape is RoundRectangle with {radius} radius"; the ctor seeds it with ChangeCornerRadius(0).
// This port reproduces that exactly: each button rebuilds the border's round_rectangle(i_shape) at the new
// uniform radius and refreshes the readout (the StrokeShape is owned via shared_ptr — set a fresh instance
// to retrigger the mapper, the border.hpp invalidation rule).
//
// The page OWNS its whole element tree (the gallery_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the C# <Style> resources (BorderStyle / StrokeShapeBindingParentBorderStyle) are applied
//         imperatively here — the port's code-first API has no XAML StaticResource Style application; each
//         Setter is reproduced as the equivalent set_* call (the documented style→setter bridge).
//   note: the "StrokeShape binded to Parent" border's CornerRadius="{Binding ... AncestorType=Border,
//         Path=Width}" is a RelativeSource AncestorType binding — a deferred XAML facility (PROFILE: XAML is
//         layer 6, behind the code-first API). The visual intent (a rounded border whose corner radius
//         tracks its own width) is reproduced with a fixed radius matching the C# resolved-width look; the
//         live binding is left as best-effort + this note, never invented.
//   note: the "Headline" StaticResource caption style (a shared FontSize/weight) is applied as a plain
//         caption label — the headless-safe equivalent of the StaticResource label style.
//   note: the C# StackLayout Margin="12" is a layout-options/margin facility the port's view base does not
//         expose; the spacing (12) is reproduced, the outer margin is omitted (the same note grid_page.hpp
//         records for HorizontalOptions). HeightRequest/Padding/StrokeThickness all map 1:1.

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/border.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class border_styles_page
    {
    public:
        border_styles_page()
        {
            page_.set_title("Border using Styles");
            stack_.set_spacing(12); // C# StackLayout Spacing="12" (Margin="12" — see note)

            // --- "RoundRectangle 10,10,10,10" — the C# BorderStyle.
            caption(style_caption_, "RoundRectangle 10,10,10,10");
            apply_border_style(styled_border_, 10.0);
            stack_.add(styled_border_);

            // --- "StrokeShape binded to Parent" — the parent-width-bound RoundRectangle (note).
            caption(bound_caption_, "StrokeShape binded to Parent");
            // The C# style binds CornerRadius to the parent Border's Width; with no explicit WidthRequest the
            // resolved width drives the radius. Reproduced with a fixed radius here (the live binding is the
            // deferred XAML facility — see note), the rest of the style 1:1.
            apply_border_style(bound_border_, 20.0);
            stack_.add(bound_border_);

            // --- "Update StrokeShape" — the runtime-mutated border (UpdateStrokeShapeBorder).
            caption(update_caption_, "Update StrokeShape");
            update_border_.set_width_request(400);
            update_border_.set_height_request(200);
            update_border_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow)); // Background="Yellow"
            update_border_.set_stroke(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red)); // Stroke="Red"
            update_border_.set_stroke_thickness(2);                                          // StrokeThickness="2"
            update_border_.set_content(update_info_);                                        // hosts the readout label
            stack_.add(update_border_);

            // --- the two CornerRadius buttons (OnIncreaseCornerRadius / OnDecreaseCornerRadius).
            increase_button_.set_text("Increase CornerRadius");
            increase_button_.clicked.connect([this] { change_corner_radius(10); });
            stack_.add(increase_button_);

            decrease_button_.set_text("Decrease CornerRadius");
            decrease_button_.clicked.connect([this] { change_corner_radius(-10); });
            stack_.add(decrease_button_);

            // C# ctor seeds the StrokeShape + info label with ChangeCornerRadius(0).
            change_corner_radius(0);

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the stack's children + the update border's content
        // first, then the stack, then the page) so each parent can host its child's native view, then re-host
        // the tree built in the ctor (gallery_attach.hpp). The generic lambda preserves each member's concrete
        // static type — attach_handler keys on the static type, so an i_view& parameter would erase it.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, style_caption_, "style_caption_");
            gallery_attach_one(app, styled_border_, "styled_border_");
            gallery_attach_one(app, bound_caption_, "bound_caption_");
            gallery_attach_one(app, bound_border_, "bound_border_");
            gallery_attach_one(app, update_caption_, "update_caption_");
            gallery_attach_one(app, update_info_, "update_info_");
            gallery_attach_one(app, update_border_, "update_border_");
            gallery_attach_one(app, increase_button_, "increase_button_");
            gallery_attach_one(app, decrease_button_, "decrease_button_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now: the
            // update border hosts its info label, the stack hosts its children, the page hosts the stack.
            gallery_rehost_content(update_border_); // border hosts update_info_
            gallery_rehost_layout(stack_);          // stack hosts captions + borders + buttons
            gallery_rehost_content(page_);          // page hosts the stack
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::border& styled_border()
        {
            return styled_border_;
        }
        [[nodiscard]] maui::controls::border& bound_border()
        {
            return bound_border_;
        }
        [[nodiscard]] maui::controls::border& update_border()
        {
            return update_border_;
        }
        [[nodiscard]] maui::controls::label& update_info()
        {
            return update_info_;
        }
        [[nodiscard]] maui::controls::button& increase_button()
        {
            return increase_button_;
        }
        [[nodiscard]] maui::controls::button& decrease_button()
        {
            return decrease_button_;
        }

    private:
        // The C# ChangeCornerRadius: clamp radius to >= 0, rebuild the border's RoundRectangle stroke shape at
        // the new uniform radius, and refresh the info readout.
        void change_corner_radius(double delta)
        {
            current_radius_ = std::max(0.0, current_radius_ + delta);
            update_border_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(current_radius_));

            char text[72];
            std::snprintf(text, sizeof(text), "Border.StrokeShape is RoundRectangle with %g radius", current_radius_);
            update_info_.set_text(text);
        }

        // Apply the shared C# Border <Style> setters (RoundRectangle stroke shape, red stroke 1, height 55,
        // padding 10, transparent background) at the given uniform corner radius.
        static void apply_border_style(maui::controls::border& target, double corner_radius)
        {
            target.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(corner_radius));
            target.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
            target.set_stroke_thickness(1);
            target.set_height_request(55);
            target.set_padding(maui::core::thickness(10));
            target.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::transparent));
        }

        // One "Headline"-style caption label above a border section.
        void caption(maui::controls::label& text, const char* value)
        {
            text.set_text(value);
            stack_.add(text);
        }

        double current_radius_ = 10.0; // the update border's live RoundRectangle radius (C# ctor seed via 0-delta)

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;
        maui::controls::label style_caption_;
        maui::controls::border styled_border_;
        maui::controls::label bound_caption_;
        maui::controls::border bound_border_;
        maui::controls::label update_caption_;
        maui::controls::border update_border_;
        maui::controls::label update_info_; // the UpdateStrokeShapeInfo readout, hosted by update_border_
        maui::controls::button increase_button_;
        maui::controls::button decrease_button_;
    };
} // namespace maui::samples
