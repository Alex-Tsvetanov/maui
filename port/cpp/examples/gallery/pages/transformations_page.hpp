#pragma once
// maui::samples::transformations_page — ports TransformationsPage.xaml (+ .xaml.cs)
//
// The MAUI TransformationsPage drives a single target view's render transforms from a column of
// knobs: Sliders for Scale / ScaleX / ScaleY (Maximum 10) and Rotation / RotationX / RotationY
// (Maximum 360), and Steppers for AnchorX / AnchorY (Increment 0.5, range -1..2). Each knob is
// two-way bound to the target's corresponding property, and a per-row Label echoes the live value
// (the XAML's `{Binding Value, StringFormat=...}`).
//
// This code-first port keeps that structure: a target_ button ("SCALE AND ROTATE") plus, per
// transform, a readout label + an input control whose value_changed pushes onto the target and
// refreshes the label — reproducing the two-way binding seam with explicit handlers (no XAML
// binding engine, layer 6). Two extra TranslationX/TranslationY sliders round out the transform
// family the view exposes.
//
// Demonstrated (all headless-safe maui:: view transform setters):
//   Scale, ScaleX, ScaleY, Rotation, RotationX, RotationY (sliders) + AnchorX, AnchorY (steppers)
//   + TranslationX, TranslationY (sliders).
//
// note: VisualElement/View exposes no Skew property (neither does MAUI), so the "skew" hint is not
// applicable and intentionally omitted.
//
// Self-contained (the value_controls_page pattern): the page OWNS its whole element tree, exposes
// page().

#include <cstdio>
#include <locale>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

namespace maui::samples
{
    class transformations_page
    {
    public:
        transformations_page()
        {
            page_.set_title("Transformations");
            stack_.set_spacing(6);

            // The target whose transforms every knob drives (the XAML's "SCALE AND ROTATE" button).
            target_.set_text("SCALE AND ROTATE");

            // ---- Scale (Maximum 10, default 1) ----
            scale_slider_.set_maximum(10);
            scale_slider_.set_value(1);
            scale_slider_.value_changed.connect([this](double, double v) {
                target_.set_scale(v);
                update_readout(scale_readout_, "Scale", v, 1);
            });
            update_readout(scale_readout_, "Scale", target_.scale(), 1);

            // ---- ScaleX (Maximum 10, default 1) ----
            scale_x_slider_.set_maximum(10);
            scale_x_slider_.set_value(1);
            scale_x_slider_.value_changed.connect([this](double, double v) {
                target_.set_scale_x(v);
                update_readout(scale_x_readout_, "ScaleX", v, 1);
            });
            update_readout(scale_x_readout_, "ScaleX", target_.scale_x(), 1);

            // ---- ScaleY (Maximum 10, default 1) ----
            scale_y_slider_.set_maximum(10);
            scale_y_slider_.set_value(1);
            scale_y_slider_.value_changed.connect([this](double, double v) {
                target_.set_scale_y(v);
                update_readout(scale_y_readout_, "ScaleY", v, 1);
            });
            update_readout(scale_y_readout_, "ScaleY", target_.scale_y(), 1);

            // ---- Rotation (Maximum 360) ----
            rotation_slider_.set_maximum(360);
            rotation_slider_.value_changed.connect([this](double, double v) {
                target_.set_rotation(v);
                update_readout(rotation_readout_, "Rotation", v, 0);
            });
            update_readout(rotation_readout_, "Rotation", target_.rotation(), 0);

            // ---- RotationX (Maximum 360) ----
            rotation_x_slider_.set_maximum(360);
            rotation_x_slider_.value_changed.connect([this](double, double v) {
                target_.set_rotation_x(v);
                update_readout(rotation_x_readout_, "RotationX", v, 0);
            });
            update_readout(rotation_x_readout_, "RotationX", target_.rotation_x(), 0);

            // ---- RotationY (Maximum 360) ----
            rotation_y_slider_.set_maximum(360);
            rotation_y_slider_.value_changed.connect([this](double, double v) {
                target_.set_rotation_y(v);
                update_readout(rotation_y_readout_, "RotationY", v, 0);
            });
            update_readout(rotation_y_readout_, "RotationY", target_.rotation_y(), 0);

            // ---- AnchorX (Stepper, Increment 0.5, -1..2, default 0.5) ----
            anchor_x_stepper_.set_minimum(-1);
            anchor_x_stepper_.set_maximum(2);
            anchor_x_stepper_.set_increment(0.5);
            anchor_x_stepper_.set_value(0.5);
            anchor_x_stepper_.value_changed.connect([this](double, double v) {
                target_.set_anchor_x(v);
                update_readout(anchor_x_readout_, "AnchorX", v, 1);
            });
            update_readout(anchor_x_readout_, "AnchorX", target_.anchor_x(), 1);

            // ---- AnchorY (Stepper, Increment 0.5, -1..2, default 0.5) ----
            anchor_y_stepper_.set_minimum(-1);
            anchor_y_stepper_.set_maximum(2);
            anchor_y_stepper_.set_increment(0.5);
            anchor_y_stepper_.set_value(0.5);
            anchor_y_stepper_.value_changed.connect([this](double, double v) {
                target_.set_anchor_y(v);
                update_readout(anchor_y_readout_, "AnchorY", v, 1);
            });
            update_readout(anchor_y_readout_, "AnchorY", target_.anchor_y(), 1);

            // ---- TranslationX (slider, -100..100) — the transform family's translation seam. ----
            translation_x_slider_.set_minimum(-100);
            translation_x_slider_.set_maximum(100);
            translation_x_slider_.value_changed.connect([this](double, double v) {
                target_.set_translation_x(v);
                update_readout(translation_x_readout_, "TranslationX", v, 0);
            });
            update_readout(translation_x_readout_, "TranslationX", target_.translation_x(), 0);

            // ---- TranslationY (slider, -100..100) ----
            translation_y_slider_.set_minimum(-100);
            translation_y_slider_.set_maximum(100);
            translation_y_slider_.value_changed.connect([this](double, double v) {
                target_.set_translation_y(v);
                update_readout(translation_y_readout_, "TranslationY", v, 0);
            });
            update_readout(translation_y_readout_, "TranslationY", target_.translation_y(), 0);

            // ---------------- assemble: target first, then each [readout, knob] row ----------------
            stack_.add(target_);

            stack_.add(scale_readout_);
            stack_.add(scale_slider_);
            stack_.add(scale_x_readout_);
            stack_.add(scale_x_slider_);
            stack_.add(scale_y_readout_);
            stack_.add(scale_y_slider_);

            stack_.add(rotation_readout_);
            stack_.add(rotation_slider_);
            stack_.add(rotation_x_readout_);
            stack_.add(rotation_x_slider_);
            stack_.add(rotation_y_readout_);
            stack_.add(rotation_y_slider_);

            stack_.add(anchor_x_readout_);
            stack_.add(anchor_x_stepper_);
            stack_.add(anchor_y_readout_);
            stack_.add(anchor_y_stepper_);

            stack_.add(translation_x_readout_);
            stack_.add(translation_x_slider_);
            stack_.add(translation_y_readout_);
            stack_.add(translation_y_slider_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main / tests.
        [[nodiscard]] maui::controls::button& target()
        {
            return target_;
        }
        [[nodiscard]] maui::controls::slider& scale_slider()
        {
            return scale_slider_;
        }
        [[nodiscard]] maui::controls::slider& rotation_slider()
        {
            return rotation_slider_;
        }
        [[nodiscard]] maui::controls::stepper& anchor_x_stepper()
        {
            return anchor_x_stepper_;
        }

    private:
        // Set `label` to "<name> = <value>" — mirroring C#'s CULTURE-SENSITIVE value.ToString("F"+decimals)
        // followed by TransformationsPage.UpdateReadout's separator-guarded trim:
        //     var s = value.ToString("F" + decimals);            // fixed digits, current-culture separator
        //     if (s.Contains('.')) s = s.TrimEnd('0').TrimEnd('.');
        // maui-compare forces no culture, so the separator is the machine's (a comma on a European locale →
        // "1,0"/"0,5"); the trim tests for a LITERAL '.', so under a comma locale it never fires and the
        // fixed-decimal output is preserved. We reproduce that exactly: format F<decimals> (C-locale dot),
        // swap the dot for the OS locale's decimal separator, then trim ONLY when that separator is '.'.
        static void update_readout(maui::controls::label& label, const char* name, double value, int decimals)
        {
            char num[32];
            (void)std::snprintf(num, sizeof(num), "%.*f", decimals, value);
            std::string s{num};

            char sep = '.';
            try
            {
                sep = std::use_facet<std::numpunct<char>>(std::locale("")).decimal_point();
            }
            catch (...)
            {
                sep = '.'; // no environment locale available — keep the invariant dot
            }
            if (sep != '.')
            {
                if (const auto dot = s.find('.'); dot != std::string::npos)
                {
                    s[dot] = sep;
                }
            }
            // MAUI's TrimEnd guard is a literal '.' test: under a non-dot separator the string has no '.',
            // so the trailing-zero trim is skipped and "1,0"/"0,5" survive (matching the ground-truth render).
            if (s.find('.') != std::string::npos)
            {
                s.erase(s.find_last_not_of('0') + 1);
                if (!s.empty() && s.back() == '.')
                {
                    s.pop_back();
                }
            }
            label.set_text(std::string(name) + " = " + s);
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::button target_;

        maui::controls::label scale_readout_;
        maui::controls::slider scale_slider_;
        maui::controls::label scale_x_readout_;
        maui::controls::slider scale_x_slider_;
        maui::controls::label scale_y_readout_;
        maui::controls::slider scale_y_slider_;

        maui::controls::label rotation_readout_;
        maui::controls::slider rotation_slider_;
        maui::controls::label rotation_x_readout_;
        maui::controls::slider rotation_x_slider_;
        maui::controls::label rotation_y_readout_;
        maui::controls::slider rotation_y_slider_;

        maui::controls::label anchor_x_readout_;
        maui::controls::stepper anchor_x_stepper_;
        maui::controls::label anchor_y_readout_;
        maui::controls::stepper anchor_y_stepper_;

        maui::controls::label translation_x_readout_;
        maui::controls::slider translation_x_slider_;
        maui::controls::label translation_y_readout_;
        maui::controls::slider translation_y_slider_;
    };
} // namespace maui::samples
