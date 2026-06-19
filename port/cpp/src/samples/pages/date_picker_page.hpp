#pragma once
// maui::samples::date_picker_page — ports DatePickerPage.xaml (+ DatePickerPage.xaml.cs).
//
// The C# DatePickerPage is a VerticalStackLayout of headline-labelled date_picker variants, each
// demonstrating one facet of the control, plus a small interactive cluster wired in code-behind:
//   Default, BackgroundColor=Blue, Background (a LinearGradientBrush), a randomizable Background
//   (Update/Clear buttons), Default-with-date (06/21/2018), Disabled, TextColor=Red, Format
//   (yyyy/MM/dd), IsFocused (an echo label), Set-to-null (null/today buttons), and IsOpen
//   (Open/Close buttons + Opened/Closed console events).
//
// This is a code-first port following the pickers_page pattern: the page OWNS its whole element tree
// as members, exposes page() and attach_handlers(maui_app). It is headless-safe — only
// cross-platform maui:: API. The code-behind logic (UpdateDatePickerBackground, the button handlers,
// the IsOpen Opened/Closed subscriptions, the IsFocused binding echo) is ported into lambdas on the
// owned controls.
//
// Interactions demonstrated (mirroring the .xaml.cs):
//   - the "select a date" date_picker drives a readout label (date_selected),
//   - Update Background paints background_date_ with a fresh pseudo-random linear gradient; Clear
//     Background removes it (OnUpdate/OnClearBackgroundButtonClicked),
//   - Set-to-null / Set-to-today set null_date_'s Date to null / today (SetDatePickerToNull/Today),
//   - Open / Close drive is_open_date_'s IsOpen; its Opened/Closed events append to the readout
//     (the C# Console.WriteLine, surfaced visibly here).
//
// Fidelity notes:
//   - BackgroundColor=Blue uses set_background with a solid_paint (the VisualElement.BackgroundColor
//     path); the gradient "Background" uses a real linear_gradient_paint (Yellow@0.1 -> Green@1.0,
//     end_point (1,0)), matching the XAML LinearGradientBrush.
//   - UpdateDatePickerBackground ports the C# Random gradient: two opaque random colors, end_point
//     (1,0), stops at offset 0 and 1. A fixed-seed std::minstd_rand keeps the headless run
//     deterministic (the visual is still a random-looking gradient, refreshed each click).
//   - "Default with date" sets Date=2018-06-21 (the C# 06/21/2018). Disabled sets IsEnabled=false.
//   - The IsFocused XAML binds a label to FocusDatePicker.IsFocused; there is no public
//     is_focused-change event on the port surface, so the echo label is seeded to "False" (the
//     initial value). note: live IsFocused echo deferred — no focus-change signal on this surface.

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
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
    class date_picker_page
    {
    public:
        date_picker_page()
        {
            page_.set_title("DatePicker");
            stack_.set_padding(maui::core::thickness(12));
            stack_.set_spacing(6);

            // ---- Default ----
            default_label_.set_text("Default");

            // ---- BackgroundColor=Blue (the VisualElement.BackgroundColor path) ----
            background_color_label_.set_text("BackgroundColor");
            background_color_date_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));

            // ---- Background: a static LinearGradientBrush (Yellow@0.1 -> Green@1.0, end_point (1,0)) ----
            gradient_label_.set_text("Background");
            gradient_date_.set_background(std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{
                    maui::graphics::gradient_stop(0.1F, maui::graphics::colors::yellow),
                    maui::graphics::gradient_stop(1.0F, maui::graphics::colors::green)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0)));

            // ---- Background (randomizable): Update / Clear buttons (the code-behind) ----
            random_background_label_.set_text("Background");
            update_background_date_background(); // the ctor calls UpdateDatePickerBackground() in C#
            update_background_button_.set_text("Update Background");
            update_background_button_.clicked.connect([this] { update_background_date_background(); });
            clear_background_button_.set_text("Clear Background");
            clear_background_button_.clicked.connect([this] { background_date_.set_background(nullptr); });

            // ---- Default with date: Date=06/21/2018 ----
            dated_label_.set_text("Default with date");
            dated_date_.set_date(maui::core::date_time(2018, 6, 21));

            // ---- Disabled: IsEnabled=false ----
            disabled_label_.set_text("Disabled");
            disabled_date_.set_is_enabled(false);

            // ---- TextColor=Red ----
            text_color_label_.set_text("TextColor");
            text_color_date_.set_text_color(maui::graphics::colors::red);

            // ---- Format=yyyy/MM/dd ----
            format_label_.set_text("Format");
            format_date_.set_format("yyyy/MM/dd");

            // ---- IsFocused: an echo label (seeded to the initial value; note above) ----
            focus_label_.set_text("IsFocused");
            is_focused_caption_.set_text("IsFocused");
            focus_result_.set_text("False");
            focus_row_.set_spacing(6);
            focus_row_.add(is_focused_caption_);
            focus_row_.add(focus_result_);

            // ---- Set to null: null / today buttons ----
            null_label_.set_text("Set to null");
            null_date_.set_date(std::nullopt); // Date="{x:Null}"
            set_null_button_.set_text("Set to null");
            set_null_button_.clicked.connect([this] { null_date_.set_date(std::nullopt); });
            set_today_button_.set_text("Set to today");
            set_today_button_.clicked.connect([this] { null_date_.set_date(maui::core::date_time::today()); });

            // ---- IsOpen: Open / Close buttons + Opened/Closed events ----
            is_open_label_.set_text("IsOpen");
            is_open_date_.opened.connect([this] { readout_.set_text("IsOpenDatePicker Opened"); });
            is_open_date_.closed.connect([this] { readout_.set_text("IsOpenDatePicker Closed"); });
            open_button_.set_text("Open DatePicker");
            open_button_.clicked.connect([this] { is_open_date_.set_is_open(true); });
            close_button_.set_text("Close DatePicker");
            close_button_.clicked.connect([this] { is_open_date_.set_is_open(false); });

            // ---- the live "select a date" picker driving the readout (the demonstrated wiring) ----
            select_label_.set_text("Select a date");
            select_date_.date_selected.connect(
                [this](const std::optional<maui::core::date_time>&, const std::optional<maui::core::date_time>& now) {
                    readout_.set_text(now ? "Selected: " + maui::core::format_date_time(*now, "yyyy/MM/dd")
                                          : std::string("Selected: (none)"));
                });
            readout_.set_text("Pick a date above");

            stack_.add(default_label_);
            stack_.add(default_date_);
            stack_.add(background_color_label_);
            stack_.add(background_color_date_);
            stack_.add(gradient_label_);
            stack_.add(gradient_date_);
            stack_.add(random_background_label_);
            stack_.add(background_date_);
            stack_.add(update_background_button_);
            stack_.add(clear_background_button_);
            stack_.add(dated_label_);
            stack_.add(dated_date_);
            stack_.add(disabled_label_);
            stack_.add(disabled_date_);
            stack_.add(text_color_label_);
            stack_.add(text_color_date_);
            stack_.add(format_label_);
            stack_.add(format_date_);
            stack_.add(focus_label_);
            stack_.add(focus_date_);
            stack_.add(focus_row_);
            stack_.add(null_label_);
            stack_.add(null_date_);
            stack_.add(set_null_button_);
            stack_.add(set_today_button_);
            stack_.add(is_open_label_);
            stack_.add(is_open_date_);
            stack_.add(open_button_);
            stack_.add(close_button_);
            stack_.add(select_label_);
            stack_.add(select_date_);
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
            one(default_date_, "default_date_");
            one(background_color_label_, "background_color_label_");
            one(background_color_date_, "background_color_date_");
            one(gradient_label_, "gradient_label_");
            one(gradient_date_, "gradient_date_");
            one(random_background_label_, "random_background_label_");
            one(background_date_, "background_date_");
            one(update_background_button_, "update_background_button_");
            one(clear_background_button_, "clear_background_button_");
            one(dated_label_, "dated_label_");
            one(dated_date_, "dated_date_");
            one(disabled_label_, "disabled_label_");
            one(disabled_date_, "disabled_date_");
            one(text_color_label_, "text_color_label_");
            one(text_color_date_, "text_color_date_");
            one(format_label_, "format_label_");
            one(format_date_, "format_date_");
            one(focus_label_, "focus_label_");
            one(focus_date_, "focus_date_");
            one(is_focused_caption_, "is_focused_caption_");
            one(focus_result_, "focus_result_");
            one(focus_row_, "focus_row_");
            one(null_label_, "null_label_");
            one(null_date_, "null_date_");
            one(set_null_button_, "set_null_button_");
            one(set_today_button_, "set_today_button_");
            one(is_open_label_, "is_open_label_");
            one(is_open_date_, "is_open_date_");
            one(open_button_, "open_button_");
            one(close_button_, "close_button_");
            one(select_label_, "select_label_");
            one(select_date_, "select_date_");
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
        // Ports DatePickerPage.UpdateDatePickerBackground: a fresh pseudo-random linear gradient between
        // two opaque colors, end_point (1,0), stops at 0 and 1. Deterministic seed keeps the headless run
        // reproducible (still visibly refreshes on each click).
        void update_background_date_background()
        {
            std::uniform_int_distribution<int> byte_dist(0, 255);
            const maui::graphics::color start =
                maui::graphics::color::from_rgba(byte_dist(rng_), byte_dist(rng_), byte_dist(rng_), 255);
            const maui::graphics::color end =
                maui::graphics::color::from_rgba(byte_dist(rng_), byte_dist(rng_), byte_dist(rng_), 255);
            background_date_.set_background(std::make_shared<maui::graphics::linear_gradient_paint>(
                std::vector<maui::graphics::gradient_stop>{maui::graphics::gradient_stop(0.0F, start),
                                                           maui::graphics::gradient_stop(1.0F, end)},
                maui::graphics::point(0, 0), maui::graphics::point(1, 0)));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_label_;
        maui::controls::date_picker default_date_;
        maui::controls::label background_color_label_;
        maui::controls::date_picker background_color_date_;
        maui::controls::label gradient_label_;
        maui::controls::date_picker gradient_date_;
        maui::controls::label random_background_label_;
        maui::controls::date_picker background_date_;
        maui::controls::button update_background_button_;
        maui::controls::button clear_background_button_;
        maui::controls::label dated_label_;
        maui::controls::date_picker dated_date_;
        maui::controls::label disabled_label_;
        maui::controls::date_picker disabled_date_;
        maui::controls::label text_color_label_;
        maui::controls::date_picker text_color_date_;
        maui::controls::label format_label_;
        maui::controls::date_picker format_date_;
        maui::controls::label focus_label_;
        maui::controls::date_picker focus_date_;
        maui::controls::horizontal_stack_layout focus_row_;
        maui::controls::label is_focused_caption_;
        maui::controls::label focus_result_;
        maui::controls::label null_label_;
        maui::controls::date_picker null_date_;
        maui::controls::button set_null_button_;
        maui::controls::button set_today_button_;
        maui::controls::label is_open_label_;
        maui::controls::date_picker is_open_date_;
        maui::controls::button open_button_;
        maui::controls::button close_button_;
        maui::controls::label select_label_;
        maui::controls::date_picker select_date_;
        maui::controls::label readout_;

        std::minstd_rand rng_{12345}; // fixed seed — headless determinism (header note)
    };
} // namespace maui::samples
