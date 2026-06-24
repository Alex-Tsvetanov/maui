#pragma once
// maui::samples::context_flyout_page — ports ContextFlyoutPage.xaml (+ ContextFlyoutPage.xaml.cs)
//
// The C# page attaches a MenuFlyout as the FlyoutBase.ContextFlyout (right-click / long-press menu) of
// several controls and wires each menu item to a handler:
//   - a Button ("Increment by 1 (or right-click me)") whose Clicked bumps a counter by 1, and whose
//     context MenuFlyout has items "Increment by 10 / 20 / 30 (disabled) / 40 (dynamic)" plus a
//     "Increment by 500" sub-item containing "Increment by 1,000 / 1,000,000". OnIncrementMenuItemClicked
//     parses each item's CommandParameter and adds it to the counter; "by 40" is a dynamic Command gated
//     on a Switch.
//   - a counter Label ({Binding CounterValue}) + a "Is dynamic menu enabled?" Label + a Switch
//     (IsToggled ⇄ IsDynamicCommandEnabled).
//   - a Label with an "Add additional menus" context flyout (add/remove top-level + sub-menu items),
//   - a Label with a "beautiful menus" color flyout (icon menu items + an "Advanced colors" sub-menu),
//   - an Entry with a custom flyout (show text in a message / add text / clear text),
//   - an Image (a 🆒 FontImageSource) with "Use Clicked event" / "Use Command and CommandParameter", and
//   - a WebView (bing.com) with "Go to MAUI repo" / "Invoke some JS".
//
// PORT MAPPING:
//   - FlyoutBase.ContextFlyout  -> view::set_context_flyout(&menu_flyout) (controls/view.hpp; the chrome
//     W1-11 surface — see chrome_page.hpp).
//   - MenuFlyout / MenuFlyoutItem / MenuFlyoutSubItem / MenuFlyoutSeparator  -> the same-named controls;
//     items are appended via menu_flyout::items().add / menu_flyout_sub_item::items().add.
//   - MenuFlyoutItem.Clicked + CommandParameter  -> the port collapses Command/ICommand on menu items
//     into the `clicked` event (menu_item.hpp: "the clicked event IS the command channel"). So each
//     increment item connects clicked to a lambda that captures its increment amount and bumps the
//     counter — the exact effect of the C# OnIncrementMenuItemClicked(int.Parse(CommandParameter)).
//   - the Switch (IsDynamicCommandEnabled)  -> toggle_switch; toggling it enables/disables the "by 40"
//     item (the dynamic Command's CanExecute) via set_is_enabled — the observable result of C#'s
//     DynamicEnabledCommand.ChangeCanExecute.
//   - the "by 30 (disabled)" item is created disabled (bbb.IsEnabled = false in the .xaml.cs).
//
// Because the gallery does NOT open native context menus, the interactions are exercised PROGRAMMATICALLY
// (each menu item's activate() is the same path a native click takes — menu_item.hpp send_clicked →
// activate → clicked) and every activation drives the visible counter / status readout the C# page shows.
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree (every control AND every menu item);
// the generic mount (app_host.hpp) attaches every owned VIEW's handler and hosts the tree. Menu items /
// flyouts / separators are NON-view items with no standalone handler, so they are deliberately excluded
// from the attach (attaching one would throw) — exactly as chrome_page.hpp excludes its menu items.
//
// note: the C# DisplayAlert side-effects (the entry / image / "add menu" / webview message boxes) have no
//       headless analog; each ported handler instead stamps the status label with the same message text,
//       the gallery's observable-readout convention.
// note: the 🆒 FontImageSource image source + the WebView's bing.com navigation are best-effort: the image
//       carries the FontImageSource and the web_view carries the URL source, but neither renders pixels
//       headless. The demonstrated feature — context menu items driving a readout — is fully exercised.
// note: KeyboardAccelerators (Alt+F on the sub-item, etc., added in the .xaml.cs) are attached to the
//       relevant items so the accelerator surface ports too, though no key dispatch happens headless.

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/font_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_separator.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard_accelerator.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class context_flyout_page
    {
    public:
        context_flyout_page()
        {
            page_.set_title("Context Flyout");
            stack_.set_spacing(10);

            build_increment_button();
            build_color_label();
            build_entry();
            build_image();
            build_web_view();

            // The counter + dynamic-state readout (the C# {Binding CounterValue} label + the dynamic flag).
            counter_label_.set_text("0");
            stack_.add(counter_label_);
            status_.set_text("Right-click a control, or its menu items are exercised programmatically");
            stack_.add(status_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Owned controls exposed for the hosting main / the headless test tree.
        [[nodiscard]] maui::controls::button& increment_button()
        {
            return increment_button_;
        }
        [[nodiscard]] maui::controls::menu_flyout& increment_menu()
        {
            return increment_menu_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& increment10()
        {
            return increment10_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& increment40_dynamic()
        {
            return increment40_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& increment1000()
        {
            return increment1000_;
        }
        [[nodiscard]] maui::controls::toggle_switch& dynamic_switch()
        {
            return dynamic_switch_;
        }
        [[nodiscard]] maui::controls::entry& entry()
        {
            return entry_;
        }
        [[nodiscard]] maui::controls::label& counter_label()
        {
            return counter_label_;
        }
        [[nodiscard]] maui::controls::label& status()
        {
            return status_;
        }
        [[nodiscard]] int count() const
        {
            return count_;
        }

    private:
        // The Button + its context MenuFlyout (Increment by 10/20/30-disabled/40-dynamic, then a
        // "by 500" sub-item with "by 1,000" / "by 1,000,000").
        void build_increment_button()
        {
            increment_button_.set_text("Increment by 1 (or right-click me)");
            increment_button_.set_background(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::dark_gray));
            increment_button_.clicked.connect([this]() { add(1); }); // OnIncrementByOneClicked

            wire_increment(increment10_, "Increment by 10", 10);
            increment10_.accelerators().push_back(
                {maui::core::keyboard_accelerator_modifiers::alt | maui::core::keyboard_accelerator_modifiers::ctrl,
                 "A"});
            wire_increment(increment20_, "Increment by 20", 20);
            increment20_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::shift, "B"});
            // "by 30 (disabled)" — bbb.IsEnabled = false in the .xaml.cs.
            wire_increment(increment30_, "Increment by 30 (disabled)", 30);
            increment30_.set_is_enabled(false);
            increment30_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::none, "C"});
            // "by 40" — the dynamic Command gated on the switch; starts disabled (switch off).
            wire_increment(increment40_, "Increment by 40 (dynamic enabled/disabled)", 40);
            increment40_.set_is_enabled(false);

            increment500_.set_text("Increment by 500");
            increment500_.clicked.connect([this]() { add(500); });
            wire_increment(increment1000_, "Increment by 1,000!", 1000);
            increment1000_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::ctrl, "t"});
            wire_increment(increment1000000_, "Increment by 1,000,000!", 1000000);
            increment1000000_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::alt, "b"});
            increment500_.items().add(increment1000_);
            increment500_.items().add(increment1000000_);

            increment_menu_.items().add(increment10_);
            increment_menu_.items().add(increment20_);
            increment_menu_.items().add(increment30_);
            increment_menu_.items().add(increment40_);
            increment_menu_.items().add(increment500_);
            increment_button_.set_context_flyout(&increment_menu_);

            stack_.add(increment_button_);

            // The "Is dynamic menu enabled?" label + the Switch (IsToggled ⇄ IsDynamicCommandEnabled). The
            // toggle enables/disables the "by 40" item — the observable result of ChangeCanExecute.
            dynamic_label_.set_text("Is dynamic menu enabled?");
            stack_.add(dynamic_label_);
            dynamic_switch_.toggled.connect([this](bool enabled) {
                increment40_.set_is_enabled(enabled);
                stamp(enabled ? "Dynamic increment enabled" : "Dynamic increment disabled");
            });
            stack_.add(dynamic_switch_);
        }

        // The "beautiful menus" color label: icon menu items (Red/Blue/Green/…), a separator, and an
        // "Advanced colors" sub-menu. Each item stamps its color name (no counter change).
        void build_color_label()
        {
            color_label_.set_text("Right-click to see beautiful menus");

            add_color_item(red_, "Red");
            add_color_item(blue_, "Blue");
            add_color_item(green_, "Green");
            color_menu_.items().add(color_separator_);
            add_color_item(orange_, "Orange");

            advanced_colors_.set_text("Advanced colors");
            add_color_item(chartreuse_, "Chartreuse");
            add_color_item(misty_rose_, "Misty Rose");
            add_color_item(medium_purple_, "Medium Purple");
            advanced_colors_.items().add(chartreuse_);
            advanced_colors_.items().add(misty_rose_);
            advanced_colors_.items().add(medium_purple_);

            color_menu_.items().add(red_);
            color_menu_.items().add(blue_);
            color_menu_.items().add(green_);
            color_menu_.items().add(color_separator_);
            color_menu_.items().add(orange_);
            color_menu_.items().add(advanced_colors_);
            color_label_.set_context_flyout(&color_menu_);
            stack_.add(color_label_);
        }

        // The Entry with a custom context flyout: show / add / clear text. Each stamps the readout with
        // the same message the C# DisplayAlert / mutation produces.
        void build_entry()
        {
            entry_.set_placeholder("Has a custom context menu");

            entry_show_.set_text("Show text in a message");
            entry_show_.clicked.connect([this]() { stamp("The entry's text is: " + std::string(entry_.text())); });
            entry_add_.set_text("Add some text");
            entry_add_.clicked.connect([this]() {
                entry_.set_text(std::string(entry_.text()) + " more text!");
                stamp("Added text to entry");
            });
            entry_clear_.set_text("Clear all text");
            entry_clear_.clicked.connect([this]() {
                entry_.set_text("");
                stamp("Cleared entry text");
            });

            entry_menu_.items().add(entry_show_);
            entry_menu_.items().add(entry_add_);
            entry_menu_.items().add(entry_clear_);
            entry_.set_context_flyout(&entry_menu_);
            stack_.add(entry_);
        }

        // The Image (a 🆒 FontImageSource) with "Use Clicked event" / "Use Command and CommandParameter".
        // The C# command item carries CommandParameter="some value"; the port collapses command-as-clicked,
        // so its handler stamps the same parameter-bearing message.
        void build_image()
        {
            image_.set_source(std::make_shared<maui::controls::font_image_source>(
                "\xF0\x9F\x86\x92", maui::core::font::of_size("Arial", 50), maui::graphics::colors::medium_purple));
            image_.set_maximum_height_request(200);
            image_.set_maximum_width_request(200);

            image_clicked_.set_text("Use Clicked event");
            image_clicked_.clicked.connect([this]() { stamp("The image's context menu was clicked"); });
            image_command_.set_text("Use Command and CommandParameter");
            image_command_.clicked.connect(
                [this]() { stamp("The image's context menu was clicked via a command with parameter: some value"); });
            image_menu_.items().add(image_clicked_);
            image_menu_.items().add(image_command_);
            image_.set_context_flyout(&image_menu_);
            stack_.add(image_);
        }

        // The WebView (bing.com) with "Go to MAUI repo" / "Invoke some JS". The first re-sources the
        // webview to the MAUI repo (C# OnWebViewGoToSiteClicked); the second stamps the JS message.
        void build_web_view()
        {
            web_view_.set_source(std::string("https://bing.com"));
            web_view_.set_minimum_height_request(400);

            web_goto_.set_text("Go to MAUI repo");
            web_goto_.clicked.connect([this]() {
                web_view_.set_source(std::string("https://github.com/dotnet/maui"));
                stamp("WebView navigated to the MAUI repo");
            });
            web_js_.set_text("Invoke some JS");
            web_js_.clicked.connect([this]() { stamp("Invoked JS: alert('help, i'm being invoked!')"); });
            web_menu_.items().add(web_goto_);
            web_menu_.items().add(web_separator_);
            web_menu_.items().add(web_js_);
            web_view_.set_context_flyout(&web_menu_);
            stack_.add(web_view_);
        }

        // Wire one increment menu item: set its text and connect its click to add(amount) — the port's
        // command-as-clicked channel for the C# OnIncrementMenuItemClicked(int.Parse(CommandParameter)).
        void wire_increment(maui::controls::menu_flyout_item& item, const char* text, int amount)
        {
            item.set_text(text);
            item.clicked.connect([this, amount]() { add(amount); });
        }

        // One color menu item: set text + stamp its color name on click (C# "beautiful menus" — the items
        // have no handler in XAML, but a readout makes the click observable in the gallery).
        void add_color_item(maui::controls::menu_flyout_item& item, const char* name)
        {
            item.set_text(name);
            item.clicked.connect([this, name]() { stamp(std::string("Picked color: ") + name); });
        }

        // The C# counter: add to count, format with thousands separators (CounterValue => ToString("N0")),
        // write it into the counter label, and stamp the status.
        void add(int amount)
        {
            count_ += amount;
            counter_label_.set_text(format_thousands(count_));
            stamp("Counter is now " + format_thousands(count_));
        }

        void stamp(const std::string& what)
        {
            status_.set_text("Last: " + what);
        }

        // C# ToString("N0") — group digits in threes with commas (the counter display format).
        static std::string format_thousands(long long value)
        {
            const bool negative = value < 0;
            std::string digits = std::to_string(negative ? -value : value);
            std::string out;
            int since = 0;
            for (auto it = digits.rbegin(); it != digits.rend(); ++it)
            {
                if (since != 0 && since % 3 == 0)
                {
                    out.push_back(',');
                }
                out.push_back(*it);
                ++since;
            }
            if (negative)
            {
                out.push_back('-');
            }
            return {out.rbegin(), out.rend()};
        }

        int count_ = 0;

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;

        // ---- the increment button + its flyout ----
        maui::controls::button increment_button_;
        maui::controls::menu_flyout increment_menu_;
        maui::controls::menu_flyout_item increment10_;
        maui::controls::menu_flyout_item increment20_;
        maui::controls::menu_flyout_item increment30_;
        maui::controls::menu_flyout_item increment40_;
        maui::controls::menu_flyout_sub_item increment500_;
        maui::controls::menu_flyout_item increment1000_;
        maui::controls::menu_flyout_item increment1000000_;
        maui::controls::label dynamic_label_;
        maui::controls::toggle_switch dynamic_switch_;

        // ---- the color label + its flyout ----
        maui::controls::label color_label_;
        maui::controls::menu_flyout color_menu_;
        maui::controls::menu_flyout_item red_;
        maui::controls::menu_flyout_item blue_;
        maui::controls::menu_flyout_item green_;
        maui::controls::menu_flyout_separator color_separator_;
        maui::controls::menu_flyout_item orange_;
        maui::controls::menu_flyout_sub_item advanced_colors_;
        maui::controls::menu_flyout_item chartreuse_;
        maui::controls::menu_flyout_item misty_rose_;
        maui::controls::menu_flyout_item medium_purple_;

        // ---- the entry + its flyout ----
        maui::controls::entry entry_;
        maui::controls::menu_flyout entry_menu_;
        maui::controls::menu_flyout_item entry_show_;
        maui::controls::menu_flyout_item entry_add_;
        maui::controls::menu_flyout_item entry_clear_;

        // ---- the image + its flyout ----
        maui::controls::image image_;
        maui::controls::menu_flyout image_menu_;
        maui::controls::menu_flyout_item image_clicked_;
        maui::controls::menu_flyout_item image_command_;

        // ---- the web view + its flyout ----
        maui::controls::web_view web_view_;
        maui::controls::menu_flyout web_menu_;
        maui::controls::menu_flyout_item web_goto_;
        maui::controls::menu_flyout_separator web_separator_;
        maui::controls::menu_flyout_item web_js_;

        // ---- the readouts ----
        maui::controls::label counter_label_;
        maui::controls::label status_;
    };
} // namespace maui::samples
