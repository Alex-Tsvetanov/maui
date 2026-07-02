#pragma once
// maui::samples::pointer_gesture_page — ports PointerGestureGalleryPage.xaml (+ .xaml.cs)
//                                       (Maui.Controls.Sample.Pages.PointerGestureGalleryPage).
//
// The C# page is a single StackLayout of labels carrying PointerGestureRecognizers that report hover /
// press / release and the live pointer position. It demonstrates three things:
//   1. The full PointerGestureRecognizer event family on a target label (pgrLabel): PointerEntered /
//      PointerExited / PointerMoved / PointerPressed / PointerReleased — each updates the label's text
//      and background as you hover, press, and release it, plus three position readouts off PointerMoved
//      (position relative to the sender, to the window (null), and to a specific label).
//   2. A second hover-only label (hoverLabel) wired to Entered/Exited/Moved with its own position
//      readouts (the Buttons-default Primary hover path).
//   3. A command-driven label (colorfulHoverLabel): a SINGLE Command is bound to both
//      PointerEnteredCommand and PointerExitedCommand with different CommandParameters (Colors.Green on
//      enter, Colors.Black on exit) — the command recolors the label's TextColor. This is the U-CMD
//      PointerEnteredCommand/CommandParameter path the recognizer ports.
//
// This code-first port reproduces all three sections. Each recognizer is a real
// maui::controls::pointer_gesture_recognizer added to its label's GestureRecognizers collection (the
// View seam). On a device the platform pointer manager drives them; the headless backend has no native
// input, so the page's mount hook finishes with one deterministic synthetic drive per section through each
// recognizer's send_pointer_* seam (exactly how the gesture unit tests exercise them), leaving every
// readout reacting in a static capture.
//
// Demonstrated (the recognizers exist; the synthetic drive updates the readouts):
//   - pgr section: entered -> "hovering", moved -> three position readouts, pressed -> "pressing",
//     released -> "releasing", exited -> "hover again" (+ the pgrLabel background recolor sequence).
//   - hover section: entered/moved/exited drive hoverLabel + its position readouts.
//   - command section: enter runs the shared command with parameter Colors.Green (text turns green),
//     exit runs it with Colors.Black (text turns black) — proving the one-command/two-parameter wiring.
//
// The page OWNS its whole element tree (the gestures_page / value_controls_page pattern): public
// page().
//
// note: PointerEventArgs.GetPosition(relativeTo) is narrowed in the port to a single stored
//       view-relative position (the position carried on send_pointer_*). The C# three-readout split
//       (relative to sender / window / a label) has no separate coordinate seam headless, so the port's
//       three readouts echo the one carried position, labeled to match the C# intent; the per-target
//       coordinate transform is the documented gap (pointer_gesture_recognizer.hpp).

#include <any>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>

#include "maui/controls/command.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/i_command.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class pointer_gesture_page
    {
    public:
        pointer_gesture_page()
        {
            page_.set_title("Pointer gesture");
            stack_.set_spacing(8);

            // ---- section 1: the full pointer family (pgrLabel) -----------------------------------------
            pgr_label_.set_text("Hover, press, and release me!");
            pgr_label_.set_font(maui::core::font::system_font_of_size(24.0)); // C# pgrLabel FontSize="24"
            pgr_position_label_.set_text("Hover above label to reveal pointer position");
            pgr_position_to_window_.set_text("");
            pgr_position_to_this_label_.set_text("");

            pgr_->pointer_entered.connect([this](const maui::controls::pointer_event_args&) {
                pgr_label_.set_text("Thanks for hovering me! Now press me!");
                pgr_label_.set_background(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::pale_green));
            });
            pgr_->pointer_exited.connect([this](const maui::controls::pointer_event_args&) {
                pgr_label_.set_text("Hover me again!");
                pgr_position_label_.set_text("Hover above label to reveal pointer position again");
                pgr_label_.set_background(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::transparent));
            });
            pgr_->pointer_moved.connect(
                [this](const maui::controls::pointer_event_args& e) { report_position(e, "pgr"); });
            pgr_->pointer_pressed.connect([this](const maui::controls::pointer_event_args&) {
                pgr_label_.set_text("Thanks for pressing me! Now release me!");
                pgr_label_.set_background(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::sky_blue));
            });
            pgr_->pointer_released.connect([this](const maui::controls::pointer_event_args&) {
                pgr_label_.set_text("Thanks for releasing me! Press me again or leave me!");
                pgr_label_.set_background(
                    std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::yellow));
            });
            pgr_label_.gesture_recognizers().add(pgr_);

            // ---- section 2: hover-only label (hoverLabel) ----------------------------------------------
            hover_label_.set_text("Hover me!");
            hover_label_.set_font(maui::core::font::system_font_of_size(24.0)); // C# hoverLabel FontSize="24"
            position_label_.set_text("Hover above label to reveal pointer position");
            position_to_window_.set_text("");
            position_to_this_label_.set_text("");

            hover_->pointer_entered.connect([this](const maui::controls::pointer_event_args&) {
                hover_label_.set_text("Thanks for hovering me!");
            });
            hover_->pointer_exited.connect([this](const maui::controls::pointer_event_args&) {
                hover_label_.set_text("Hover me again!");
                position_label_.set_text("Hover above label to reveal pointer position again");
            });
            hover_->pointer_moved.connect(
                [this](const maui::controls::pointer_event_args& e) { report_position(e, "hover"); });
            hover_label_.gesture_recognizers().add(hover_);

            // ---- section 3: the command-driven colorful label (colorfulHoverLabel) ---------------------
            // A SINGLE Command bound to BOTH PointerEnteredCommand and PointerExitedCommand, distinguished
            // only by the CommandParameter (Green on enter, Black on exit). The command recolors the label
            // text — the C# HandleHoverCommand(Color). U-CMD command-before-event path.
            colorful_hover_label_.set_text("Hover me green!");
            colorful_hover_label_.set_font(
                maui::core::font::system_font_of_size(24.0)); // C# colorfulHoverLabel FontSize="24"
            hover_command_ = std::make_shared<maui::controls::command>([this](const std::any& parameter) {
                if (const std::optional<maui::graphics::color> color =
                        maui::core::try_unbox<maui::graphics::color>(parameter))
                {
                    colorful_hover_label_.set_text_color(*color);
                    last_command_color_ = *color;
                }
            });
            colorful_gesture_->set_pointer_entered_command(hover_command_);
            colorful_gesture_->set_pointer_entered_command_parameter(std::any(maui::graphics::colors::green));
            colorful_gesture_->set_pointer_exited_command(hover_command_);
            colorful_gesture_->set_pointer_exited_command_parameter(std::any(maui::graphics::colors::black));
            colorful_hover_label_.gesture_recognizers().add(colorful_gesture_);

            colorful_hint_.set_text("Hover above label to make it turn green");

            // ---- assemble (the C# StackLayout order) ---------------------------------------------------
            stack_.add(pgr_label_);
            stack_.add(pgr_position_label_);
            stack_.add(pgr_position_to_window_);
            stack_.add(pgr_position_to_this_label_);
            stack_.add(hover_label_);
            stack_.add(position_label_);
            stack_.add(position_to_window_);
            stack_.add(position_to_this_label_);
            stack_.add(colorful_hover_label_);
            stack_.add(colorful_hint_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Headless has no native input, so drive one deterministic synthetic
        // pointer sequence per section so every readout reflects the wiring in a static capture. All per-control
        // attach + re-host plumbing is now the generic mount's job.
        void on_mounted(maui::hosting::maui_app& /*app*/)
        {
            drive_synthetic_pointer();
        }

        // The owned views + recognizers, exposed for the hosting main / headless tests.
        [[nodiscard]] maui::controls::label& pgr_label()
        {
            return pgr_label_;
        }
        [[nodiscard]] maui::controls::label& colorful_hover_label()
        {
            return colorful_hover_label_;
        }
        [[nodiscard]] maui::graphics::color last_command_color() const
        {
            return last_command_color_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pointer_gesture_recognizer>& pgr()
        {
            return pgr_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pointer_gesture_recognizer>& hover()
        {
            return hover_;
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::pointer_gesture_recognizer>& colorful_gesture()
        {
            return colorful_gesture_;
        }

        // One deterministic drive per section through each recognizer's send_pointer_* seam (the same path
        // the platform bridge + the gesture unit tests use). Leaves every readout on the last event.
        void drive_synthetic_pointer()
        {
            // Section 1 (pgrLabel): entered -> moved -> pressed -> released -> exited.
            pgr_->send_pointer_entered(pgr_label_, maui::graphics::point{20, 20});
            pgr_->send_pointer_moved(pgr_label_, maui::graphics::point{40, 30});
            pgr_->send_pointer_pressed(pgr_label_, maui::graphics::point{40, 30});
            pgr_->send_pointer_released(pgr_label_, maui::graphics::point{40, 30});
            // Leave pgrLabel showing the released text + yellow background (the press/release end state).

            // Section 2 (hoverLabel): entered -> moved -> exited.
            hover_->send_pointer_entered(hover_label_, maui::graphics::point{10, 10});
            hover_->send_pointer_moved(hover_label_, maui::graphics::point{25, 18});
            // Leave hoverLabel hovered with a position readout.

            // Section 3 (colorfulHoverLabel): enter runs the command with Green, exit with Black. Drive
            // enter LAST so the static capture shows the label green (the section's headline "turn green").
            colorful_gesture_->send_pointer_exited(colorful_hover_label_, maui::graphics::point{0, 0});
            colorful_gesture_->send_pointer_entered(colorful_hover_label_, maui::graphics::point{15, 15});
        }

    private:
        // The three position readouts off PointerMoved. The port carries ONE position on the event
        // (GetPosition(relativeTo) is narrowed — see the header note); the three labels echo it under the
        // C# captions so the readout structure matches.
        void report_position(const maui::controls::pointer_event_args& e, const char* which)
        {
            maui::graphics::point pos = e.position.value_or(maui::graphics::point{0, 0});
            char text[96];

            (void)std::snprintf(text, sizeof(text), "Pointer position is at: {%.0f, %.0f}", pos.x, pos.y);
            (which[0] == 'p' ? pgr_position_label_ : position_label_).set_text(text);

            (void)std::snprintf(text, sizeof(text), "Pointer position inside window: {%.0f, %.0f}", pos.x, pos.y);
            (which[0] == 'p' ? pgr_position_to_window_ : position_to_window_).set_text(text);

            (void)std::snprintf(text, sizeof(text), "Pointer position relative to this label: {%.0f, %.0f}", pos.x,
                                pos.y);
            (which[0] == 'p' ? pgr_position_to_this_label_ : position_to_this_label_).set_text(text);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        // Section 1 (the full pointer family).
        maui::controls::label pgr_label_;
        maui::controls::label pgr_position_label_;
        maui::controls::label pgr_position_to_window_;
        maui::controls::label pgr_position_to_this_label_;

        // Section 2 (hover-only).
        maui::controls::label hover_label_;
        maui::controls::label position_label_;
        maui::controls::label position_to_window_;
        maui::controls::label position_to_this_label_;

        // Section 3 (command-driven).
        maui::controls::label colorful_hover_label_;
        maui::controls::label colorful_hint_;
        std::shared_ptr<maui::controls::command> hover_command_;
        maui::graphics::color last_command_color_{}; // the last color the shared command applied (test seam)

        // The recognizers (the collection co-owns them via shared_ptr; we keep strong refs for the
        // synthetic drives + the test accessors).
        std::shared_ptr<maui::controls::pointer_gesture_recognizer> pgr_ =
            std::make_shared<maui::controls::pointer_gesture_recognizer>();
        std::shared_ptr<maui::controls::pointer_gesture_recognizer> hover_ =
            std::make_shared<maui::controls::pointer_gesture_recognizer>();
        std::shared_ptr<maui::controls::pointer_gesture_recognizer> colorful_gesture_ =
            std::make_shared<maui::controls::pointer_gesture_recognizer>();
    };
} // namespace maui::samples
