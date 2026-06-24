#pragma once
// maui::samples::animation_page — ports AnimationPage.xaml (+ AnimationPage.xaml.cs).
//
// The C# gallery page wires three buttons over a single target image:
//   - "Start Animation"  -> a chained sequence of TranslateToAsync hops (await-gated so a cancel
//      mid-chain stops the rest), disabling Start + enabling Cancel for the run,
//   - "Start Custom Animation" -> a composite Animation (scale-up SpringIn / rotate 0..360 / scale-down
//      SpringOut) committed by hand,
//   - "Cancel Animation" -> ViewExtensions.CancelAnimations on the target, re-enabling Start.
//
// This port reproduces all three on the headless-safe `maui::` surface, code-first:
//   - the chained sequence uses translate_to with the completion callback (the port's Task<bool>
//     stand-in: on_finished(canceled)) to gate each hop on the previous one not being canceled — the
//     faithful translation of the C# `if (!isCancelled)` ladder,
//   - the custom run builds a maui::controls::animation composite (scale-up SpringIn over [0,0.5],
//     rotate over [0,1], scale-down SpringOut over [0.5,1]) and commits it on the target,
//   - cancel calls maui::controls::cancel_animations(target) and restores the button enabled-states.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly. The target here is a label stand-in for the C#
// dotnet_bot.png image: a label is a view<…> too, so every transform setter / animation extension applies
// to it identically and it needs no image asset to render in the headless gallery (note: the C# original
// animates an Image — swap the label for an image + set_source to mirror the asset exactly).

#include <string>

#include "maui/animations/easing.hpp"
#include "maui/controls/animation.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view_extensions.hpp"

namespace maui::samples
{
    class animation_page
    {
    public:
        animation_page()
        {
            page_.set_title("Animations");
            stack_.set_spacing(12);

            // The animation target — a label stand-in for the C# dotnet_bot.png Image (a label is a
            // view<…>, so fade/scale/rotate/translate + cancel apply to it identically).
            target_.set_text(".NET bot (animation target)");

            start_button_.set_text("Start Animation");
            start_button_.clicked.connect([this] { on_start_animation(); });

            custom_button_.set_text("Start Custom Animation");
            custom_button_.clicked.connect([this] { on_start_custom_animation(); });

            cancel_button_.set_text("Cancel Animation");
            cancel_button_.set_is_enabled(false); // C# IsEnabled="false" at rest
            cancel_button_.clicked.connect([this] { on_cancel_animation(); });

            stack_.add(target_);
            stack_.add(start_button_);
            stack_.add(custom_button_);
            stack_.add(cancel_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Owned controls, exposed for tests / the hosting main.
        [[nodiscard]] maui::controls::label& target()
        {
            return target_;
        }
        [[nodiscard]] maui::controls::button& start_button()
        {
            return start_button_;
        }
        [[nodiscard]] maui::controls::button& custom_button()
        {
            return custom_button_;
        }
        [[nodiscard]] maui::controls::button& cancel_button()
        {
            return cancel_button_;
        }

    private:
        // C# SetIsEnabledButtonState(startState, cancelState).
        void set_button_state(bool start_state, bool cancel_state)
        {
            start_button_.set_is_enabled(start_state);
            cancel_button_.set_is_enabled(cancel_state);
        }

        // C# OnStartAnimationButtonClicked: a chain of TranslateToAsync hops, each gated on the previous
        // not being canceled. translate_to's on_finished(canceled) is the port's Task<bool> stand-in, so
        // the `if (!isCancelled)` ladder becomes nested completion callbacks. Re-enables Start at the end.
        void on_start_animation()
        {
            set_button_state(false, true); // disable Start, enable Cancel for the run

            maui::controls::translate_to(target_, -100, 0, 1000, {}, [this](bool canceled) {
                if (canceled)
                {
                    return; // a cancel mid-chain stops the rest (the C# !isCancelled gate)
                }
                maui::controls::translate_to(target_, -100, -100, 1000, {}, [this](bool canceled2) {
                    if (canceled2)
                    {
                        return;
                    }
                    maui::controls::translate_to(target_, 100, 100, 2000, {}, [this](bool canceled3) {
                        if (canceled3)
                        {
                            return;
                        }
                        maui::controls::translate_to(target_, 0, 100, 1000, {}, [this](bool canceled4) {
                            if (canceled4)
                            {
                                return;
                            }
                            maui::controls::translate_to(target_, 0, 0, 1000, {}, [this](bool /*canceled5*/) {
                                set_button_state(true, false); // chain done -> restore rest state
                            });
                        });
                    });
                });
            });
        }

        // C# OnStartCustomAnimationButtonClicked: a parent Animation with three children — scale-up
        // (SpringIn) over [0,0.5], rotate 0..360 over [0,1], scale-down (SpringOut) over [0.5,1] — committed
        // by hand over 4000 ms. The finished callback restores the rest button-state.
        void on_start_custom_animation()
        {
            auto parent = std::make_shared<maui::controls::animation>();
            auto scale_up = std::make_shared<maui::controls::animation>([this](double v) { target_.set_scale(v); }, 1.0,
                                                                        2.0, maui::animations::easing::spring_in());
            auto rotate =
                std::make_shared<maui::controls::animation>([this](double v) { target_.set_rotation(v); }, 0.0, 360.0);
            auto scale_down = std::make_shared<maui::controls::animation>(
                [this](double v) { target_.set_scale(v); }, 2.0, 1.0, maui::animations::easing::spring_out());

            parent->add(0.0, 0.5, scale_up);
            parent->add(0.0, 1.0, rotate);
            parent->add(0.5, 1.0, scale_down);

            parent->commit(target_, "custom_animation", 16, 4000, {},
                           [this](double /*final_value*/, bool /*canceled*/) { set_button_state(true, false); });
        }

        // C# OnCancelAnimationButtonClicked: ViewExtensions.CancelAnimations(target) + restore rest state.
        void on_cancel_animation()
        {
            maui::controls::cancel_animations(target_);
            set_button_state(true, false);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label target_;
        maui::controls::button start_button_;
        maui::controls::button custom_button_;
        maui::controls::button cancel_button_;
    };
} // namespace maui::samples
