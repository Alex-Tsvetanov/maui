#pragma once
// maui::samples::title_bar_page — ports TitleBarPage.xaml
//
// A self-contained, code-first demo of the TitleBar control. It mirrors the C# gallery page
// (Pages/Controls/TitleBarPage.xaml): a 2-column grid of option panels that mutate a custom TitleBar
// installed on the window — the left panel sets the TitleBar's text + content, the right panel its
// colors and window placement. The C# page builds a TitleBar (Title/Subtitle/IsVisible bound to a
// view-model) and, as checkboxes/entries change, sets its Icon, LeadingContent, Content,
// TrailingContent, HeightRequest, BackgroundColor and ForegroundColor.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same wiring directly.
//
// SCOPE — the port's title_bar is the REDUCED control (title_bar.hpp): Title, Subtitle and a single
// custom Content view. The full C# TitleBar (a TemplatedView with Icon, LeadingContent,
// TrailingContent, ForegroundColor, IsVisible, HeightRequest, BackgroundColor and visual states) is
// out of scope, and C# maps Window.TitleBar on Windows + Mac Catalyst ONLY. So this page ports the
// cross-platform surface the port exposes and keeps the remaining option controls present + wired to
// the closest in-scope effect, each with a // note: explaining the deviation — never inventing a
// surface that doesn't exist.
//
// WIRED to the real title_bar surface (the C# behavior reproduced exactly):
//   - the Title entry drives title_bar.set_title (C# Text="{Binding Title, Mode=TwoWay}" → TitleBar.Title);
//   - the Subtitle entry drives title_bar.set_subtitle (C# Subtitle binding);
//   - the Content checkbox sets/clears a SearchBar as the title_bar's Content (C#
//     ContentCheckBox_CheckedChanged: a "Search" SearchBar, height 32 — the port owns it; title_bar
//     Content is non-owning).
//
// FAITHFUL BEST-EFFORT + note (the C# toggle is preserved but its effect is out of the port's surface):
//   note: "Set Icon" — C# sets TitleBar.Icon ("tb_appicon.png"). The port's title_bar has no Icon, so
//         the checkbox is wired to a readout instead of an image (documented; no Icon surface).
//   note: "Leading Content" / "Trailing Content" — C# sets TitleBar.LeadingContent (a Button) /
//         TrailingContent (a circular Border avatar). The port's title_bar has a single Content slot,
//         no leading/trailing slots, so these toggles update the readout (documented).
//   note: "Tall TitleBar" — C# sets TitleBar.HeightRequest (48/32, or 60/36 on Mac Catalyst). The
//         port's title_bar has no HeightRequest, so this toggle updates the readout (documented).
//   note: "Show TitleBar" — C# binds IsVisible. The port's title_bar has no IsVisible, so this toggle
//         updates the readout (documented).
//   note: "Set Color" / "Set Foreground Color" — C# parses the entry text (Color.TryParse) into
//         TitleBar.BackgroundColor / ForegroundColor. The port's title_bar exposes neither, so the
//         buttons parse the text with color::try_parse and report success/failure to the readout (the
//         parse step is real; only the unavailable color application is collapsed — documented).
//   note: "Toggle Title Bar On Window" / "Toggle Has Navigation Bar" / "Push New TitleBar Page" — C#
//         mutate Window.TitleBar / Shell+NavigationPage nav-bar visibility / push a new page. Those
//         window/navigation hosts are out of scope for a single code-first page, so each button reports
//         its intent to the readout (documented).

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/title_bar.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class title_bar_page
    {
    public:
        title_bar_page()
        {
            page_.set_title("TitleBarPage");

            // The custom title bar the page configures (C# _customTitleBar). Installed on the window by
            // the hosting main; not part of the content tree (title_bar is an element with no handler).
            bar_.set_title("Title");
            bar_.set_subtitle("Subtitle");

            // Grid: RowDefinitions="Auto,*", ColumnDefinitions="*,*". The two option panels sit in
            // columns 0 and 1 of row 0.
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());

            build_content_options();
            build_color_options();

            // Place the two panels (Grid.Column 0 / 1).
            grid_.add(content_panel_);
            grid_.set_row(content_panel_, 0);
            grid_.set_column(content_panel_, 0);
            grid_.add(color_panel_);
            grid_.set_row(color_panel_, 0);
            grid_.set_column(color_panel_, 1);

            page_.set_content(grid_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The custom title bar the hosting main installs on the window (C# Window.TitleBar = _customTitleBar).
        [[nodiscard]] maui::controls::title_bar& bar()
        {
            return bar_;
        }

        // Exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::entry& title_entry()
        {
            return title_entry_;
        }
        [[nodiscard]] maui::controls::check_box& content_check()
        {
            return content_check_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }

    private:
        // ---- left panel: content options (C# first VerticalStackLayout) ----
        void build_content_options()
        {
            content_panel_.set_spacing(16);

            content_heading_.set_text("Content Options");
            content_heading_.set_font(maui::core::font::system_font_of_size(24));
            content_panel_.add(content_heading_);

            // "Set Icon" — C# sets TitleBar.Icon; the port has no Icon (note) → readout.
            check_row(set_icon_row_, set_icon_check_, set_icon_label_, "Set Icon");
            set_icon_check_.checked_changed.connect([this](bool on) {
                readout_.set_text(on ? "Icon: tb_appicon.png (no Icon surface in port)" : "Icon cleared");
            });
            content_panel_.add(set_icon_row_);

            // Title entry — drives title_bar.set_title (C# Text="{Binding Title, Mode=TwoWay}").
            title_entry_.set_placeholder("Title Text");
            title_entry_.set_text("Title");
            title_entry_.set_width_request(120);
            title_entry_.text_changed.connect(
                [this](const std::string& /*old*/, const std::string& value) { bar_.set_title(value); });
            content_panel_.add(title_entry_);

            // Subtitle entry — drives title_bar.set_subtitle (C# Subtitle binding).
            subtitle_entry_.set_placeholder("Subtitle Text");
            subtitle_entry_.set_text("Subtitle");
            subtitle_entry_.set_width_request(120);
            subtitle_entry_.text_changed.connect(
                [this](const std::string& /*old*/, const std::string& value) { bar_.set_subtitle(value); });
            content_panel_.add(subtitle_entry_);

            // "Leading Content" — C# sets a Button LeadingContent; the port has no leading slot (note).
            check_row(leading_row_, leading_check_, leading_label_, "Leading Content");
            leading_check_.checked_changed.connect([this](bool on) {
                readout_.set_text(on ? "Leading content on (no leading slot in port)" : "Leading content off");
            });
            content_panel_.add(leading_row_);

            // "Content" — C# sets a "Search" SearchBar as TitleBar.Content. The port supports a single
            // Content slot, so this is the ONE option wired to the real surface.
            check_row(content_row_, content_check_, content_label_, "Content");
            content_check_.checked_changed.connect([this](bool on) {
                if (on)
                {
                    content_search_.set_placeholder("Search");
                    content_search_.set_height_request(32);
                    bar_.set_content(&content_search_);
                    readout_.set_text("Content: Search bar set on title bar");
                }
                else
                {
                    bar_.set_content(nullptr);
                    readout_.set_text("Content cleared");
                }
            });
            content_panel_.add(content_row_);

            // "Trailing Content" — C# sets a circular avatar Border TrailingContent; no trailing slot (note).
            check_row(trailing_row_, trailing_check_, trailing_label_, "Trailing Content");
            trailing_check_.checked_changed.connect([this](bool on) {
                readout_.set_text(on ? "Trailing content on (no trailing slot in port)" : "Trailing content off");
            });
            content_panel_.add(trailing_row_);

            // "Tall TitleBar" — C# sets HeightRequest; the port has no HeightRequest (note).
            check_row(tall_row_, tall_check_, tall_label_, "Tall TitleBar");
            tall_check_.checked_changed.connect([this](bool on) {
                readout_.set_text(on ? "Tall mode on (no HeightRequest surface in port)" : "Tall mode off");
            });
            content_panel_.add(tall_row_);

            // "Show TitleBar" — C# binds IsVisible; the port has no IsVisible (note).
            check_row(show_row_, show_check_, show_label_, "Show TitleBar");
            show_check_.set_is_checked(true);
            show_check_.checked_changed.connect([this](bool on) {
                readout_.set_text(on ? "Show title bar (no IsVisible surface in port)" : "Hide title bar");
            });
            content_panel_.add(show_row_);
        }

        // ---- right panel: color options (C# second VerticalStackLayout) ----
        void build_color_options()
        {
            color_panel_.set_spacing(16);

            color_heading_.set_text("Color Options");
            color_heading_.set_font(maui::core::font::system_font_of_size(24));
            color_panel_.add(color_heading_);

            // "Set Color" — C# Color.TryParse(text) → TitleBar.BackgroundColor. The parse is real; the
            // port's title_bar has no BackgroundColor, so success/failure is reported (note).
            color_entry_.set_placeholder("Green");
            color_entry_.set_width_request(120);
            color_button_.set_text("Set Color");
            color_button_.clicked.connect([this]() {
                maui::graphics::color parsed;
                if (maui::graphics::color::try_parse(color_entry_.text(), parsed))
                {
                    readout_.set_text("Background color parsed (no BackgroundColor surface in port)");
                }
                else
                {
                    readout_.set_text("Background color: parse failed");
                }
            });
            color_row_.set_spacing(8);
            color_row_.add(color_entry_);
            color_row_.add(color_button_);
            color_panel_.add(color_row_);

            // "Set Foreground Color" — C# Color.TryParse(text) → TitleBar.ForegroundColor (note).
            foreground_entry_.set_placeholder("Green");
            foreground_entry_.set_width_request(120);
            foreground_button_.set_text("Set Foreground Color");
            foreground_button_.clicked.connect([this]() {
                maui::graphics::color parsed;
                if (maui::graphics::color::try_parse(foreground_entry_.text(), parsed))
                {
                    readout_.set_text("Foreground color parsed (no ForegroundColor surface in port)");
                }
                else
                {
                    readout_.set_text("Foreground color: parse failed");
                }
            });
            foreground_row_.set_spacing(8);
            foreground_row_.add(foreground_entry_);
            foreground_row_.add(foreground_button_);
            color_panel_.add(foreground_row_);

            // The three window/navigation action buttons (C# Window.TitleBar / nav-bar toggles / page
            // push) — out of scope for a single code-first page; each reports its intent (note).
            toggle_window_button_.set_text("Toggle Title Bar On Window");
            toggle_window_button_.clicked.connect(
                [this]() { readout_.set_text("Toggle title bar on window (window host out of scope)"); });
            toggle_navbar_button_.set_text("Toggle Has Navigation Bar");
            toggle_navbar_button_.clicked.connect(
                [this]() { readout_.set_text("Toggle navigation bar (nav host out of scope)"); });
            push_button_.set_text("Push New TitleBar Page");
            push_button_.clicked.connect(
                [this]() { readout_.set_text("Push new TitleBar page (navigation host out of scope)"); });
            action_row_.set_spacing(8);
            action_row_.add(toggle_window_button_);
            action_row_.add(toggle_navbar_button_);
            action_row_.add(push_button_);
            color_panel_.add(action_row_);

            readout_.set_text("TitleBar: Title / Subtitle / Content are live");
            color_panel_.add(readout_);
        }

        // One "<CheckBox/> <Label/>" row (the C# HorizontalStackLayout pattern around each toggle).
        void check_row(maui::controls::horizontal_stack_layout& row, maui::controls::check_box& box,
                       maui::controls::label& text, const char* caption)
        {
            box.set_is_checked(false);
            text.set_text(caption);
            row.add(box);
            row.add(text);
        }

        maui::controls::content_page page_;
        maui::controls::grid grid_;
        maui::controls::title_bar bar_;
        maui::controls::search_bar content_search_; // the bar's Content while the checkbox is checked

        // Left panel (content options).
        maui::controls::vertical_stack_layout content_panel_;
        maui::controls::label content_heading_;
        maui::controls::horizontal_stack_layout set_icon_row_;
        maui::controls::check_box set_icon_check_;
        maui::controls::label set_icon_label_;
        maui::controls::entry title_entry_;
        maui::controls::entry subtitle_entry_;
        maui::controls::horizontal_stack_layout leading_row_;
        maui::controls::check_box leading_check_;
        maui::controls::label leading_label_;
        maui::controls::horizontal_stack_layout content_row_;
        maui::controls::check_box content_check_;
        maui::controls::label content_label_;
        maui::controls::horizontal_stack_layout trailing_row_;
        maui::controls::check_box trailing_check_;
        maui::controls::label trailing_label_;
        maui::controls::horizontal_stack_layout tall_row_;
        maui::controls::check_box tall_check_;
        maui::controls::label tall_label_;
        maui::controls::horizontal_stack_layout show_row_;
        maui::controls::check_box show_check_;
        maui::controls::label show_label_;

        // Right panel (color options).
        maui::controls::vertical_stack_layout color_panel_;
        maui::controls::label color_heading_;
        maui::controls::horizontal_stack_layout color_row_;
        maui::controls::entry color_entry_;
        maui::controls::button color_button_;
        maui::controls::horizontal_stack_layout foreground_row_;
        maui::controls::entry foreground_entry_;
        maui::controls::button foreground_button_;
        maui::controls::horizontal_stack_layout action_row_;
        maui::controls::button toggle_window_button_;
        maui::controls::button toggle_navbar_button_;
        maui::controls::button push_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
