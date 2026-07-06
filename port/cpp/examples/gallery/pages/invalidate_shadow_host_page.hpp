#pragma once
// maui::samples::invalidate_shadow_host_page — ports InvalidateShadowHostPage.xaml
//
// A self-contained, code-first demo that a shadow re-applies (invalidates) when its host's size changes,
// mirroring the C# core gallery page (Pages/Core/ShadowGalleries/InvalidateShadowHostPage.xaml + .xaml.cs).
// A Border with a red Shadow is the host; "Update Host Size" resizes the host (OnUpdateHostSizeClicked sets
// MinimumHeight/Width to a random 100-300), proving the shadow re-draws against the new bounds. Below the
// button, X/Y offset sliders, a radius slider, and an opacity slider drive the shadow's scalar params
// (each echoing its value into a readout Label, the StringFormat bindings).
//
// What the C# page binds vs what the port reproduces:
//   - Shadow.Brush  = Red (a fixed solid brush in the XAML, not slider-driven).
//   - Shadow.Radius <= ShadowRadiusSlider.Value   -> set_radius(value).
//   - Shadow.Opacity<= ShadowOpacitySlider.Value  -> set_opacity(value).
//   - Shadow.Offset <= Point(OffsetXSlider, OffsetYSlider) in UpdateShadowOffset (the ctor + on-change).
//   - "Update Host Size" sets the host's Minimum{Height,Width}Request to a pseudo-random 100-300, which
//     re-lays-out the host and re-applies its shadow (the invalidation under test). The port re-applies a
//     fresh shadow on the resize too, so the seam is observable headless.
// As in shadow_playground_page, each "apply" rebuilds a fresh maui::core::shadow and set_shadow()s it on
// the host so view::set_shadow fires the change and the view_mapper re-runs map_shadow (the binding layer
// is M5, so the page wires the events directly).
//
// Surface notes (best-effort): the random 100-300 resize is reproduced with a small deterministic stepping
// counter rather than <random> so the headless run is reproducible — the demonstrated behavior (the host
// changing size and the shadow re-applying) is identical. // note: deterministic size stepping stands in
// for System.Random in the C# OnUpdateHostSizeClicked.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same wiring directly.

#include <cstdio>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/shadow.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class invalidate_shadow_host_page
    {
    public:
        invalidate_shadow_host_page()
        {
            page_.set_title("Invalidate Shadow Host");
            stack_.set_spacing(8);

            // ---- the host whose resize re-applies the shadow ----
            host_.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::green));
            host_.set_stroke_thickness(4);
            host_.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::white));
            host_.set_minimum_height_request(host_size_);
            host_.set_minimum_width_request(host_size_);

            // ---- the controls column ----
            host_header_.set_text("Host");
            update_size_button_.set_text("Update Host Size");
            update_size_button_.clicked.connect([this] { update_host_size(); });

            shadow_header_.set_text("Shadow");

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

            // Radius slider [0, 20], default 10.
            radius_label_.set_text("Radius: 10");
            radius_.set_minimum(0);
            radius_.set_maximum(20);
            radius_.set_value(10);
            radius_.value_changed.connect([this](double /*old*/, double value) {
                update_label(radius_label_, "Radius: %.0f", value);
                apply_shadow();
            });

            // Opacity slider [0, 1], default 1.
            opacity_label_.set_text("Opacity: 1.00");
            opacity_.set_minimum(0);
            opacity_.set_maximum(1);
            opacity_.set_value(1);
            opacity_.value_changed.connect([this](double /*old*/, double value) {
                update_label(opacity_label_, "Opacity: %.2f", value);
                apply_shadow();
            });

            stack_.add(host_header_);
            stack_.add(update_size_button_);
            stack_.add(shadow_header_);
            stack_.add(offset_x_label_);
            stack_.add(offset_x_);
            stack_.add(offset_y_label_);
            stack_.add(offset_y_);
            stack_.add(radius_label_);
            stack_.add(radius_);
            stack_.add(opacity_label_);
            stack_.add(opacity_);
            stack_.add(host_); // the shadowed host sits at the bottom of the page

            page_.set_content(stack_);

            // The canonical shared invalidate_shadow_host.xaml declares only the resting Border chrome —
            // the <Shadow> element itself is outside the supported dialect (loader gap), so the twin (and
            // MAUI's actual render) shows NO shadow at rest despite the sliders' non-zero default values.
            // apply_shadow() now runs only from the interactive handlers, matching that at-rest state.
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::border& host()
        {
            return host_;
        }
        [[nodiscard]] maui::controls::button& update_size_button()
        {
            return update_size_button_;
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
        [[nodiscard]] double host_size() const
        {
            return host_size_;
        }

    private:
        // OnUpdateHostSizeClicked — step the host to a new size in [100, 300] and re-apply the shadow,
        // proving the shadow invalidates against the host's new bounds. Deterministic stepping (50px,
        // wrapping) stands in for System.Random so the headless run is reproducible.
        void update_host_size()
        {
            host_size_ += 50.0;
            if (host_size_ > 300.0)
            {
                host_size_ = 100.0;
            }
            host_.set_minimum_height_request(host_size_);
            host_.set_minimum_width_request(host_size_);
            apply_shadow(); // the host resized — re-apply the shadow against the new bounds.
        }

        // Build a fresh red shadow from the current slider values and set it on the host. A new instance is
        // set each time so view::set_shadow detects the change and re-runs map_shadow (the M5 binding seam).
        void apply_shadow()
        {
            auto value = std::make_shared<maui::core::shadow>();
            value->set_color(maui::graphics::colors::red);
            value->set_radius(radius_.value());
            value->set_opacity(opacity_.value());
            value->set_offset(maui::graphics::point(offset_x_.value(), offset_y_.value()));
            host_.set_shadow(std::move(value));
        }

        // Echo a slider's value into its readout label (the XAML StringFormat bindings).
        static void update_label(maui::controls::label& target, const char* format, double value)
        {
            char text[48];
            std::snprintf(text, sizeof(text), format, value);
            target.set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label host_header_;
        maui::controls::button update_size_button_;
        maui::controls::label shadow_header_;
        maui::controls::label offset_x_label_;
        maui::controls::slider offset_x_;
        maui::controls::label offset_y_label_;
        maui::controls::slider offset_y_;
        maui::controls::label radius_label_;
        maui::controls::slider radius_;
        maui::controls::label opacity_label_;
        maui::controls::slider opacity_;
        maui::controls::border host_;

        double host_size_ = 300.0; // the host's current Minimum{Height,Width}Request (XAML default 300)
    };
} // namespace maui::samples
