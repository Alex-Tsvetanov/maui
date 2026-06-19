#pragma once
// maui::samples::time_picker_page — ports TimePickerPage.xaml (+ TimePickerPage.xaml.cs).
//
// The C# TimePickerPage is the time analogue of DatePickerPage: a VerticalStackLayout of
// headline-labelled time_picker variants plus a code-behind interactive cluster:
//   Default, BackgroundColor=Blue, Background (a LinearGradientBrush), a randomizable Background
//   (Update/Clear buttons), Default-with-time (4:15:26), Disabled, TextColor=Green, Format (hh:mm),
//   IsFocused (an echo label), Set-to-null (null/now buttons), and IsOpen (Open/Close buttons +
//   Opened/Closed console events).
//
// This is a code-first port following the pickers_page pattern: the page OWNS its whole element tree
// as members, exposes page() and attach_handlers(maui_app). Headless-safe — only cross-platform
// maui:: API. The code-behind logic (UpdateTimePickerBackground, the button handlers, the IsOpen
// Opened/Closed subscriptions, the IsFocused echo) is ported into lambdas on the owned controls.
//
// Interactions demonstrated (mirroring the .xaml.cs):
//   - the "select a time" time_picker drives a readout label (time_selected),
//   - Update Background paints background_time_ with a fresh pseudo-random linear gradient; Clear
//     Background removes it (OnUpdate/OnClearBackgroundButtonClicked),
//   - Set-to-null / Set-to-now set null_time_'s Time to null / now (SetTimePickerToNull/Now),
//   - Open / Close drive is_open_time_'s IsOpen; its Opened/Closed events update the readout.
//
// Fidelity notes:
//   - BackgroundColor=Blue uses set_background with a solid_paint; the gradient "Background" uses a
//     real linear_gradient_paint (Yellow@0.1 -> Green@1.0, end_point (1,0)). (The C# XAML actually
//     writes the gradient under a stray <DatePicker.Background> tag inside the <TimePicker> — a
//     copy-paste artifact in the sample; the intent is the TimePicker's Background, ported faithfully.)
//   - UpdateTimePickerBackground ports the C# Random gradient: two opaque random colors, end_point
//     (1,0), stops at 0 and 1; a fixed-seed std::minstd_rand keeps the headless run deterministic.
//   - "Default with time" sets Time=04:15:26. Disabled sets IsEnabled=false. TextColor=Green.
//     Format="hh:mm".
//   - SetTimePickerToNow uses date_time::now().time_of_day() as a time_span (the C# DateTime.Now.TimeOfDay).
//   - note: live IsFocused echo deferred — no focus-change signal on this surface; the echo label is
//     seeded to the initial "False".

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class time_picker_page
    {
    public:
        time_picker_page()
        {
            page_.set_title("TimePicker");
            stack_.set_padding(maui::core::thickness(12));
            stack_.set_spacing(6);

            // ---- Default ----
            default_label_.set_text("Default");

            // ---- BackgroundColor=Blue (the VisualElement.BackgroundColor path) ----
            background_color_label_.set_text("BackgroundColor");
            background_color_time_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));

            // ---- Background: a static LinearGradientBrush (Yellow@0.1 -> Green@1.0, end_point (1,0)) ----
            gradient_label_.set_text("Background");
            gradient_time_.set_background(std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{
                    maui::graphics::gradient_stop(0.1F, maui::graphics::colors::yellow),
                    maui::graphics::gradient_stop(1.0F, maui::graphics::colors::green)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0)));

            // ---- Background (randomizable): Update / Clear buttons (the code-behind) ----
            random_background_label_.set_text("Background");
            update_background_time_background(); // the ctor calls UpdateTimePickerBackground() in C#
            update_background_button_.set_text("Update Background");
            update_background_button_.clicked.connect([this] { update_background_time_background(); });
            clear_background_button_.set_text("Clear Background");
            clear_background_button_.clicked.connect([this] { background_time_.set_background(nullptr); });

            // ---- Default with time: Time=4:15:26 ----
            timed_label_.set_text("Default with time");
            timed_time_.set_time(maui::core::time_span(4, 15, 26));

            // ---- Disabled: IsEnabled=false ----
            disabled_label_.set_text("Disabled");
            disabled_time_.set_is_enabled(false);

            // ---- TextColor=Green ----
            text_color_label_.set_text("TextColor");
            text_color_time_.set_text_color(maui::graphics::colors::green);

            // ---- Format=hh:mm ----
            format_label_.set_text("Format");
            format_time_.set_format("hh:mm");

            // ---- IsFocused: an echo label (seeded to the initial value; note above) ----
            focus_label_.set_text("IsFocused");
            is_focused_caption_.set_text("IsFocused");
            focus_result_.set_text("False");
            focus_row_.set_spacing(6);
            focus_row_.add(is_focused_caption_);
            focus_row_.add(focus_result_);

            // ---- Set to null: null / now buttons ----
            null_label_.set_text("Set to null");
            null_time_.set_time(std::nullopt); // Time="{x:Null}"
            set_null_button_.set_text("Set to null");
            set_null_button_.clicked.connect([this] { null_time_.set_time(std::nullopt); });
            set_now_button_.set_text("Set to now");
            set_now_button_.clicked.connect([this] {
                const auto now = maui::core::date_time::now().time_of_day();
                null_time_.set_time(maui::core::time_span(now));
            });

            // ---- IsOpen: Open / Close buttons + Opened/Closed events ----
            is_open_label_.set_text("IsOpen");
            is_open_time_.opened.connect([this] { readout_.set_text("IsOpenTimePicker Opened"); });
            is_open_time_.closed.connect([this] { readout_.set_text("IsOpenTimePicker Closed"); });
            open_button_.set_text("Open TimePicker");
            open_button_.clicked.connect([this] { is_open_time_.set_is_open(true); });
            close_button_.set_text("Close TimePicker");
            close_button_.clicked.connect([this] { is_open_time_.set_is_open(false); });

            // ---- the live "select a time" picker driving the readout (the demonstrated wiring) ----
            select_label_.set_text("Select a time");
            select_time_.set_format("HH:mm");
            select_time_.time_selected.connect(
                [this](const std::optional<maui::core::time_span>&, const std::optional<maui::core::time_span>& now) {
                    readout_.set_text(now ? "Selected: " + maui::core::format_time_span(*now, "HH:mm")
                                          : std::string("Selected: (none)"));
                });
            readout_.set_text("Pick a time above");

            stack_.add(default_label_);
            stack_.add(default_time_);
            stack_.add(background_color_label_);
            stack_.add(background_color_time_);
            stack_.add(gradient_label_);
            stack_.add(gradient_time_);
            stack_.add(random_background_label_);
            stack_.add(background_time_);
            stack_.add(update_background_button_);
            stack_.add(clear_background_button_);
            stack_.add(timed_label_);
            stack_.add(timed_time_);
            stack_.add(disabled_label_);
            stack_.add(disabled_time_);
            stack_.add(text_color_label_);
            stack_.add(text_color_time_);
            stack_.add(format_label_);
            stack_.add(format_time_);
            stack_.add(focus_label_);
            stack_.add(focus_time_);
            stack_.add(focus_row_);
            stack_.add(null_label_);
            stack_.add(null_time_);
            stack_.add(set_null_button_);
            stack_.add(set_now_button_);
            stack_.add(is_open_label_);
            stack_.add(is_open_time_);
            stack_.add(open_button_);
            stack_.add(close_button_);
            stack_.add(select_label_);
            stack_.add(select_time_);
            stack_.add(readout_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves -> rows -> stack -> scroll -> page), then
        // re-host the tree built in the ctor. EXCLUDES non-view items (paints).
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
            one(default_time_, "default_time_");
            one(background_color_label_, "background_color_label_");
            one(background_color_time_, "background_color_time_");
            one(gradient_label_, "gradient_label_");
            one(gradient_time_, "gradient_time_");
            one(random_background_label_, "random_background_label_");
            one(background_time_, "background_time_");
            one(update_background_button_, "update_background_button_");
            one(clear_background_button_, "clear_background_button_");
            one(timed_label_, "timed_label_");
            one(timed_time_, "timed_time_");
            one(disabled_label_, "disabled_label_");
            one(disabled_time_, "disabled_time_");
            one(text_color_label_, "text_color_label_");
            one(text_color_time_, "text_color_time_");
            one(format_label_, "format_label_");
            one(format_time_, "format_time_");
            one(focus_label_, "focus_label_");
            one(focus_time_, "focus_time_");
            one(is_focused_caption_, "is_focused_caption_");
            one(focus_result_, "focus_result_");
            one(focus_row_, "focus_row_");
            one(null_label_, "null_label_");
            one(null_time_, "null_time_");
            one(set_null_button_, "set_null_button_");
            one(set_now_button_, "set_now_button_");
            one(is_open_label_, "is_open_label_");
            one(is_open_time_, "is_open_time_");
            one(open_button_, "open_button_");
            one(close_button_, "close_button_");
            one(select_label_, "select_label_");
            one(select_time_, "select_time_");
            one(readout_, "readout_");
            one(stack_, "stack_");
            one(scroll_, "scroll_");
            one(page_, "page_");

            gallery_rehost_layout(focus_row_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroll_);
            gallery_rehost_content(page_);
        }

    private:
        // Ports TimePickerPage.UpdateTimePickerBackground: a fresh pseudo-random linear gradient between two
        // opaque colors, end_point (1,0), stops at 0 and 1. Deterministic seed for headless reproducibility.
        void update_background_time_background()
        {
            std::uniform_int_distribution<int> byte_dist(0, 255);
            const maui::graphics::color start =
                maui::graphics::color::from_rgba(byte_dist(rng_), byte_dist(rng_), byte_dist(rng_), 255);
            const maui::graphics::color end =
                maui::graphics::color::from_rgba(byte_dist(rng_), byte_dist(rng_), byte_dist(rng_), 255);
            background_time_.set_background(std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{maui::graphics::gradient_stop(0.0F, start),
                                                           maui::graphics::gradient_stop(1.0F, end)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0)));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_label_;
        maui::controls::time_picker default_time_;
        maui::controls::label background_color_label_;
        maui::controls::time_picker background_color_time_;
        maui::controls::label gradient_label_;
        maui::controls::time_picker gradient_time_;
        maui::controls::label random_background_label_;
        maui::controls::time_picker background_time_;
        maui::controls::button update_background_button_;
        maui::controls::button clear_background_button_;
        maui::controls::label timed_label_;
        maui::controls::time_picker timed_time_;
        maui::controls::label disabled_label_;
        maui::controls::time_picker disabled_time_;
        maui::controls::label text_color_label_;
        maui::controls::time_picker text_color_time_;
        maui::controls::label format_label_;
        maui::controls::time_picker format_time_;
        maui::controls::label focus_label_;
        maui::controls::time_picker focus_time_;
        maui::controls::horizontal_stack_layout focus_row_;
        maui::controls::label is_focused_caption_;
        maui::controls::label focus_result_;
        maui::controls::label null_label_;
        maui::controls::time_picker null_time_;
        maui::controls::button set_null_button_;
        maui::controls::button set_now_button_;
        maui::controls::label is_open_label_;
        maui::controls::time_picker is_open_time_;
        maui::controls::button open_button_;
        maui::controls::button close_button_;
        maui::controls::label select_label_;
        maui::controls::time_picker select_time_;
        maui::controls::label readout_;

        std::minstd_rand rng_{12345}; // fixed seed — headless determinism (header note)
    };
} // namespace maui::samples
