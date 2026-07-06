#pragma once
// maui::samples::shadow_playground_page — ports ShadowPlaygroundPage.xaml
//
// A self-contained, code-first demo of the view Shadow surface, mirroring the C# core gallery page
// (Pages/Core/ShadowGalleries/ShadowPlaygroundPage.xaml + .xaml.cs). The XAML drives four shadowed
// views (a Label, a Rectangle, a clipped Image, and a gradient RoundRectangle) from a column of
// inputs: a fill-color Entry, a shadow-color Entry, X/Y offset Sliders, a radius Slider, an opacity
// Slider, and a "Remove Shadow" Button — with each slider echoing its value into a readout Label
// (the StringFormat bindings) and the offset wired through OnShadowOffset{X,Y}Changed → UpdateShadowOffset.
//
// What the C# page binds vs what the port reproduces:
//   - Shadow.Brush  <= ShadowColor Entry text via StringToBrushConverter -> set_color(parse(hex)).
//   - Shadow.Radius <= ShadowRadiusSlider.Value                          -> set_radius(value).
//   - Shadow.Opacity<= ShadowOpacitySlider.Value                         -> set_opacity(value).
//   - Shadow.Offset <= Point(ShadowOffsetXSlider.Value, OffsetYSlider.Value) in UpdateShadowOffset.
//   - the fill Entry recolors each shadowed view's background (the Fill/Stroke bindings on the shapes).
//   - "Remove Shadow" nulls every view's Shadow (RemoveShadowClicked) — set_shadow(nullptr).
// The port re-applies the four scalar shadow params on every input change (the binding layer is M5, so
// the page wires the events directly — view::set_shadow fires the change so the view_mapper re-runs
// map_shadow). Because mutating a shared shadow in place would not re-fire the property, each "apply"
// rebuilds a fresh maui::core::shadow and set_shadow()s it on each target view.
//
// Surface notes (best-effort): the XAML demonstrates one Rectangle, a clipped Image, and a gradient
// RoundRectangle shadow brush. The headless port surface keeps this code-first and HEADLESS-SAFE, so
// the four shadow TARGETS are a Label + a box_view (the "View" rectangle) — the shadow scalar surface
// is identical on every view. The LinearGradientBrush shadow is C#-only sugar; the i_shadow paint is a
// solid_paint here. // note: gradient shadow brush + clipped-image target are out of the headless scope.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <cstdio>
#include <memory>
#include <string>

#include "maui/controls/box_view.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/shadow.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"

namespace maui::samples
{
    class shadow_playground_page
    {
    public:
        shadow_playground_page()
        {
            page_.set_title("Shadow Playground");
            stack_.set_spacing(8);

            // ---- the shadowed targets (the "Label" + "View" sections of the XAML) ----
            label_target_.set_text("Label with a Shadow");

            view_target_.set_color(maui::graphics::color::parse("#00B4DB"));
            view_target_.set_width_request(75);
            view_target_.set_height_request(50);

            // ---- the inputs (Background entry / Shadow color entry / sliders / button) ----
            background_header_.set_text("Background");
            fill_color_.set_text("#00B4DB");
            fill_color_.set_placeholder("Background Color Hex");
            // Recolor the "View" rectangle fill from the Background entry (the Fill/Stroke binding).
            fill_color_.text_changed.connect(
                [this](const std::string& /*old*/, const std::string& /*new*/) { apply_fill(); });

            shadow_header_.set_text("Shadow");

            shadow_color_label_.set_text("Shadow Color");
            shadow_color_.set_text("#FF0000");
            shadow_color_.set_placeholder("Shadow Color Hex");
            shadow_color_.text_changed.connect(
                [this](const std::string& /*old*/, const std::string& /*new*/) { apply_shadow(); });

            // Offset X / Y sliders [-20, 20], default 10 — drive Shadow.Offset (UpdateShadowOffset).
            offset_x_label_.set_text("Offset X: 10");
            offset_x_.set_minimum(-20);
            offset_x_.set_maximum(20);
            offset_x_.set_value(10);
            offset_x_.value_changed.connect([this](double /*old*/, double value) {
                update_label(offset_x_label_, "Offset X: %.0f", value);
                apply_shadow();
            });

            offset_y_label_.set_text("Offset Y: 10");
            offset_y_.set_minimum(-20);
            offset_y_.set_maximum(20);
            offset_y_.set_value(10);
            offset_y_.value_changed.connect([this](double /*old*/, double value) {
                update_label(offset_y_label_, "Offset Y: %.0f", value);
                apply_shadow();
            });

            // Radius slider [0, 20], default 10 — drives Shadow.Radius.
            radius_label_.set_text("Radius: 10");
            radius_.set_minimum(0);
            radius_.set_maximum(20);
            radius_.set_value(10);
            radius_.value_changed.connect([this](double /*old*/, double value) {
                update_label(radius_label_, "Radius: %.0f", value);
                apply_shadow();
            });

            // Opacity slider [0, 1], default 1 — drives Shadow.Opacity.
            opacity_label_.set_text("Opacity: 1.00");
            opacity_.set_minimum(0);
            opacity_.set_maximum(1);
            opacity_.set_value(1);
            opacity_.value_changed.connect([this](double /*old*/, double value) {
                update_label(opacity_label_, "Opacity: %.2f", value);
                apply_shadow();
            });

            // "Remove Shadow" — nulls every target's shadow (RemoveShadowClicked).
            remove_button_.set_text("Remove Shadow");
            remove_button_.clicked.connect([this] { remove_shadow(); });

            // Compose the input column (the scrolled StackLayout in the XAML).
            stack_.add(background_header_);
            stack_.add(fill_color_);
            stack_.add(shadow_header_);
            stack_.add(shadow_color_label_);
            stack_.add(shadow_color_);
            stack_.add(offset_x_label_);
            stack_.add(offset_x_);
            stack_.add(offset_y_label_);
            stack_.add(offset_y_);
            stack_.add(radius_label_);
            stack_.add(radius_);
            stack_.add(opacity_label_);
            stack_.add(opacity_);
            stack_.add(remove_button_);

            scroller_.set_content(stack_);

            // The page hosts the two shadowed targets above the scrolled input column.
            root_.set_spacing(12);
            root_.add(label_target_);
            root_.add(view_target_);
            root_.add(scroller_);
            page_.set_content(root_);

            // The canonical shared shadow_playground.xaml has no resting Shadow markup sink (the twin's
            // own note: Shadow is applied only through the slider/entry change handlers), so both targets
            // stay shadowless at rest, matching MAUI's actual render — only apply_fill() re-establishes
            // the BoxView's declared #00B4DB default; apply_shadow() runs on the first real input change.
            apply_fill();
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::label& label_target()
        {
            return label_target_;
        }
        [[nodiscard]] maui::controls::box_view& view_target()
        {
            return view_target_;
        }
        [[nodiscard]] maui::controls::entry& fill_color()
        {
            return fill_color_;
        }
        [[nodiscard]] maui::controls::entry& shadow_color()
        {
            return shadow_color_;
        }
        [[nodiscard]] maui::controls::slider& offset_x()
        {
            return offset_x_;
        }
        [[nodiscard]] maui::controls::slider& offset_y()
        {
            return offset_y_;
        }
        [[nodiscard]] maui::controls::slider& radius()
        {
            return radius_;
        }
        [[nodiscard]] maui::controls::slider& opacity()
        {
            return opacity_;
        }
        [[nodiscard]] maui::controls::button& remove_button()
        {
            return remove_button_;
        }
        [[nodiscard]] maui::controls::label& radius_label()
        {
            return radius_label_;
        }
        [[nodiscard]] maui::controls::label& opacity_label()
        {
            return opacity_label_;
        }

    private:
        // Build a fresh shadow from the current inputs and set it on both targets. A new instance is set
        // each time so view::set_shadow detects the change and re-runs the view_mapper's map_shadow (the
        // M5 binding seam — mutating a shared instance in place would not re-fire the property).
        void apply_shadow()
        {
            label_target_.set_shadow(make_shadow());
            view_target_.set_shadow(make_shadow());
        }

        [[nodiscard]] std::shared_ptr<maui::core::shadow> make_shadow() const
        {
            auto value = std::make_shared<maui::core::shadow>();
            value->set_color(parse_color(shadow_color_.text(), maui::graphics::color(0.0F, 0.0F, 0.0F)));
            value->set_radius(radius_.value());
            value->set_opacity(opacity_.value());
            value->set_offset(maui::graphics::point(offset_x_.value(), offset_y_.value()));
            return value;
        }

        // RemoveShadowClicked — clear the shadow on every target.
        void remove_shadow()
        {
            label_target_.set_shadow(nullptr);
            view_target_.set_shadow(nullptr);
        }

        // Recolor the "View" rectangle from the Background entry (the Fill/Stroke binding).
        void apply_fill()
        {
            view_target_.set_color(parse_color(fill_color_.text(), maui::graphics::color::parse("#00B4DB")));
        }

        // Parse a hex string from an entry; fall back to a default on an empty/invalid value (the
        // StringToBrushConverter is forgiving — an unparsable string leaves the prior brush).
        [[nodiscard]] static maui::graphics::color parse_color(std::string_view hex, maui::graphics::color fallback)
        {
            maui::graphics::color parsed;
            if (!hex.empty() && maui::graphics::color::try_parse(hex, parsed))
            {
                return parsed;
            }
            return fallback;
        }

        // Echo a slider's value into its readout label (the XAML StringFormat bindings).
        static void update_label(maui::controls::label& target, const char* format, double value)
        {
            char text[48];
            std::snprintf(text, sizeof(text), format, value);
            target.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_; // the outer Grid: targets above, inputs below

        // shadowed targets
        maui::controls::label label_target_;
        maui::controls::box_view view_target_;

        // the scrolled input column
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label background_header_;
        maui::controls::entry fill_color_;
        maui::controls::label shadow_header_;
        maui::controls::label shadow_color_label_;
        maui::controls::entry shadow_color_;
        maui::controls::label offset_x_label_;
        maui::controls::slider offset_x_;
        maui::controls::label offset_y_label_;
        maui::controls::slider offset_y_;
        maui::controls::label radius_label_;
        maui::controls::slider radius_;
        maui::controls::label opacity_label_;
        maui::controls::slider opacity_;
        maui::controls::button remove_button_;
    };
} // namespace maui::samples
