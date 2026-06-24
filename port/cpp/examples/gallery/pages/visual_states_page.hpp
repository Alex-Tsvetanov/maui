#pragma once
// maui::samples::visual_states_page — ports VisualStatesPage.xaml.
//
// The MAUI page shows the VisualStateManager swapping setters as a control moves between states:
//   - an Entry with a CommonStates group (Normal / Focused / Disabled). Normal/Disabled set BackgroundColor
//     (Lime / Pink); Focused sets FontSize=36. A DataTrigger disables it while a second entry is empty.
//   - two Buttons driven by Normal / PointerOver (and Normal / PointerOver / Pressed) groups, recoloring
//     on hover / press.
//
// Port mapping (code-first, headless-safe, faithful to the port's VSM surface — view::visual_states()):
//   - the entry's OWN visual_states() carries the CommonStates group. change_visual_state() picks
//     Disabled when !is_enabled, else Focused when focused, else Normal — exactly VisualElement
//     .ChangeVisualState — so driving the entry is as simple as set_is_focused / set_is_enabled, no manual
//     go_to_state for the system states. The Focused setter changes FontSize (font.with_size(36)), and the
//     Normal/Disabled setters change TextColor as the observable state marker.
//       note: BackgroundColor in the port is a paint (view::background), not a plain color descriptor, so a
//       setter on a `bindable_property<color>` uses TextColor here (the closest faithful color stand-in for
//       the XAML's BackgroundColor=Lime/Pink). The state-transition behavior is identical.
//   - each button's OWN visual_states() carries its group. PointerOver / Pressed are NOT system-driven, so
//     they are driven EXPLICITLY via visual_states().go_to_state(button, "PointerOver"/"Pressed"/"Normal")
//     — the port of a pointer/press gesture flipping the state. Setters change Text (the observable marker)
//     since Button.BackgroundColor is likewise a paint.
//   - the DataTrigger (disable entry-with-VSM until the enabler entry has text) is reproduced in code: the
//     enabler entry's text_changed sets the VSM entry's is_enabled, which re-drives change_visual_state.
//
// The page OWNS its whole tree; the generic mount attaches every owned view's handler.

#include <string>
#include <string_view>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/font.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class visual_states_page
    {
    public:
        visual_states_page()
        {
            page_.set_title("VisualStates");
            stack_.set_spacing(8);

            build_entry_with_vsm();
            build_enabler_entry();
            build_pointer_over_button();
            build_pressed_button();

            // StackLayout layout order, matching the XAML.
            entry_headline_.set_text("Entry with VisualStateManager:");
            enabler_headline_.set_text("Entry to enable 2nd Entry:");
            pointer_headline_.set_text("Button with Normal and PointerOver visual states:");
            pressed_headline_.set_text("Button with Normal, PointerOver, and Pressed visual states:");
            pressed_explainer_.set_text(
                "The Normal and PointerOver states for this button are the same, so the PointerOver state "
                "returns the button to its base look after a click rather than leaving it Pressed.");

            stack_.add(entry_headline_);
            stack_.add(vsm_entry_);
            stack_.add(enabler_headline_);
            stack_.add(enabler_entry_);
            stack_.add(pointer_headline_);
            stack_.add(pointer_button_);
            stack_.add(pressed_headline_);
            stack_.add(pressed_explainer_);
            stack_.add(pressed_button_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- headless drivers (observable with no backend) ----
        // Focus / unfocus the VSM entry — change_visual_state() flips it Normal<->Focused.
        void focus_vsm_entry(bool focused)
        {
            vsm_entry_.set_is_focused(focused);
        }
        // Type into the enabler entry: empties disable the VSM entry (the DataTrigger), text enables it.
        void simulate_enabler_text(std::string_view text)
        {
            const std::string old_text(enabler_entry_.text());
            enabler_entry_.set_text(std::string(text));
            enabler_entry_.send_text_changed(old_text, text);
        }
        // Drive a button's custom pointer/press states (not system-driven, so go_to_state directly).
        void set_pointer_button_state(std::string_view state)
        {
            pointer_button_.visual_states().go_to_state(pointer_button_, state);
        }
        void set_pressed_button_state(std::string_view state)
        {
            pressed_button_.visual_states().go_to_state(pressed_button_, state);
        }

        // ---- owned controls, exposed for the hosting main + tests ----
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::entry& vsm_entry()
        {
            return vsm_entry_;
        }
        [[nodiscard]] maui::controls::entry& enabler_entry()
        {
            return enabler_entry_;
        }
        [[nodiscard]] maui::controls::button& pointer_button()
        {
            return pointer_button_;
        }
        [[nodiscard]] maui::controls::button& pressed_button()
        {
            return pressed_button_;
        }

    private:
        using setter = maui::controls::setter;
        using visual_state = maui::controls::visual_state;
        using visual_state_group = maui::controls::visual_state_group;
        using common = maui::controls::common_states;

        // Entry with CommonStates (faithful to VisualStatesPage.xaml): Normal sets BackgroundColor=Lime,
        // Disabled sets BackgroundColor=Pink, Focused enlarges FontSize to 36. background_property() is a
        // paint descriptor (shared_ptr<paint>), so the setters carry a solid_paint. Driven automatically by
        // set_is_focused / set_is_enabled (change_visual_state resolves Disabled<-!enabled, Focused<-focused,
        // else Normal). NOTE: real MAUI renders this entry Lime (Normal) at REST — the XAML DataTrigger
        // (IsEnabled=False while the enabler is empty) does NOT visually swap it to the Disabled Pink at
        // startup — so the port mirrors that ground-truth render by resting in Normal.
        void build_entry_with_vsm()
        {
            auto lime_bg = std::shared_ptr<maui::graphics::paint>(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::lime));
            auto pink_bg = std::shared_ptr<maui::graphics::paint>(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::pink));

            visual_state normal{std::string(common::normal)};
            normal.add(setter::of(maui::controls::background_property(), lime_bg));

            visual_state focused{std::string(common::focused)};
            focused.add(setter::of(maui::controls::entry::font_property(), maui::core::font::system_font_of_size(36)));

            visual_state disabled{std::string(common::disabled)};
            disabled.add(setter::of(maui::controls::background_property(), pink_bg));

            visual_state_group group{"CommonStates"};
            group.add(std::move(normal));
            group.add(std::move(focused));
            group.add(std::move(disabled));
            vsm_entry_.visual_states().add_group(std::move(group));

            // Rest in Normal (Lime) to match MAUI's actual render (see note above); the enabler entry still
            // drives Disabled (Pink) on clear and set_is_focused drives Focused, as in the live demo.
            vsm_entry_.change_visual_state();
        }

        // The enabler entry: its text drives the VSM entry's IsEnabled (the DataTrigger Binding on
        // Text.Length == 0 → IsEnabled=False).
        void build_enabler_entry()
        {
            enabler_entry_.set_placeholder("Type something to enable 2nd Entry");
            enabler_connection_ = maui::core::connect_scoped(
                enabler_entry_.text_changed, [this](const std::string& /*old_value*/, const std::string& new_value) {
                    vsm_entry_.set_is_enabled(!new_value.empty()); // set_is_enabled re-drives change_visual_state
                });
        }

        // Button with Normal + PointerOver: PointerOver flips Text; Normal restores it. PointerOver is a
        // custom (non-system) state, driven explicitly via go_to_state.
        void build_pointer_over_button()
        {
            pointer_button_.set_text("Hover me to see the state change");

            visual_state normal{std::string(common::normal)};
            normal.add(
                setter::of(maui::controls::button::text_property(), std::string("Hover me to see the state change")));
            visual_state pointer_over{std::string(common::pointer_over)};
            pointer_over.add(setter::of(maui::controls::button::text_property(), std::string("Pointer is over me")));

            visual_state_group group{"CommonStates"};
            group.add(std::move(normal));
            group.add(std::move(pointer_over));
            pointer_button_.visual_states().add_group(std::move(group));
            pointer_button_.visual_states().go_to_state(pointer_button_, common::normal);
        }

        // Button with Normal + PointerOver + Pressed: Normal and PointerOver share a look; Pressed differs.
        void build_pressed_button()
        {
            pressed_button_.set_text("Click me to see the state change and revert");

            visual_state normal{std::string(common::normal)};
            normal.add(setter::of(maui::controls::button::text_property(),
                                  std::string("Click me to see the state change and revert")));
            visual_state pointer_over{std::string(common::pointer_over)};
            pointer_over.add(setter::of(maui::controls::button::text_property(),
                                        std::string("Click me to see the state change and revert")));
            visual_state pressed{std::string(common::pressed)};
            pressed.add(setter::of(maui::controls::button::text_property(), std::string("Pressed!")));

            visual_state_group group{"CommonStates"};
            group.add(std::move(normal));
            group.add(std::move(pointer_over));
            group.add(std::move(pressed));
            pressed_button_.visual_states().add_group(std::move(group));
            pressed_button_.visual_states().go_to_state(pressed_button_, common::normal);
        }

        maui::controls::content_page page_;
        maui::controls::stack_layout stack_;
        maui::controls::label entry_headline_;
        maui::controls::entry vsm_entry_;
        maui::controls::label enabler_headline_;
        maui::controls::entry enabler_entry_;
        maui::controls::label pointer_headline_;
        maui::controls::button pointer_button_;
        maui::controls::label pressed_headline_;
        maui::controls::label pressed_explainer_;
        maui::controls::button pressed_button_;
        maui::core::scoped_connection enabler_connection_;
    };
} // namespace maui::samples
