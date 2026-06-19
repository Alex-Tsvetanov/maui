#pragma once
// maui::samples::menu_bar_page — ports MenuBarPage.xaml (+ MenuBarPage.cs)
//
// The C# page declares three page-level MenuBarItems (Page.MenuBarItems — the app menu bar) and a small
// visible body:
//   - "Before File"  : "Before File Action" (accelerator "b"), "Cool item 1", a separator, and
//                       "Cool item 2 (Disabled)" (IsEnabled=false).
//   - "File"         : "Custom File" (a FontImageSource icon + Ctrl+Shift+F / Shift+F accelerators added
//                       in MenuBarPage.cs).
//   - "Custom Menu"  : "Item 1" (Ctrl/Cmd+1), a "Sub Menu 1" sub-item holding "Flyout item 1" (mic icon,
//                       Win+Shift+A) and "Flyout item 2" (coffee icon, Alt+C), then "Item 2 (Disabled)".
// The body is a VerticalStackLayout: a Label x:Name="menuLabel" ("You clicked on Menu Item:") and a
// Button "Toggle Menu Bar Item".
//
// BEHAVIOR (MenuBarPage.cs):
//   - ItemClicked(sender) → menuLabel.Text = $"You clicked on Menu Item: {mfi.Text}". Every clickable
//     flyout item (those with Clicked="ItemClicked" in XAML) routes here.
//   - OnToggleMenuBarItem: if no MenuBarItem named "Added Menu" exists, add one with two flyout items
//     ("Added Flyout Item", "Added Disabled Flyout Item" IsEnabled=false); otherwise remove it. So the
//     button toggles a fourth menu in and out of the bar.
//
// PORT MAPPING:
//   - Page.MenuBarItems  -> content_page::menu_bar_items().add(menu_bar_item) (content_page.hpp).
//   - MenuBarItem / MenuFlyoutItem / MenuFlyoutSubItem / MenuFlyoutSeparator  -> the same-named controls;
//     items appended via menu_bar_item::items().add / menu_flyout_sub_item::items().add.
//   - MenuFlyoutItem.Clicked="ItemClicked"  -> each clickable item's `clicked` connects to item_clicked,
//     which writes "You clicked on Menu Item: <text>" into the label (the port collapses the menu Command
//     channel into clicked — menu_item.hpp). IconImageSource / KeyboardAccelerators are set so those
//     surfaces port too (no pixels / key dispatch headless — see notes).
//   - OnToggleMenuBarItem  -> the button's clicked toggles the owned "Added Menu" menu_bar_item in/out of
//     page().menu_bar_items() (add when absent, remove when present), matching the C# FirstOrDefault check.
//
// Because the menu bar needs a window-chrome host the headless gallery does NOT mount (the menu bar
// materializes as the platform main menu only when a window hosts the page — chrome_page.hpp), the menu
// items are exercised PROGRAMMATICALLY (each item's activate() is the path a native menu click takes) and
// the page's own VISIBLE content (the label + toggle button) is rendered, so there is always something on
// screen plus the observable label readout.
//
// HEADLESS-SAFE maui:: API only; the page OWNS its whole element tree (every menu_bar_item AND its items)
// and attaches every owned VIEW bottom-up, then re-hosts (gallery_attach.hpp). The menu_bar_item / flyout
// item / sub_item / separator members are NON-view items with no standalone handler and are excluded from
// the attach — exactly as chrome_page.hpp excludes its menu items.
//
// note: the FontImageSource icons (Ionicons glyph on "Custom File"; mic.png / coffee.png on the sub-items)
//       are set as the items' IconImageSource so the surface ports, but render no pixels headless. The C#
//       mic.png/coffee.png are file image sources; the Ionicons glyph is a FontImageSource.
// note: KeyboardAccelerators (the per-item shortcut keys, incl. the OnPlatform Ctrl/Cmd choice) are
//       attached so the accelerator surface ports; no key dispatch occurs headless. The OnPlatform
//       modifier resolves to the cross-platform-neutral choice here (ctrl) — best-effort, the port has no
//       per-platform XAML OnPlatform evaluator in the gallery.

#include <algorithm>
#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/font_image_source.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_separator.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/keyboard_accelerator.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class menu_bar_page
    {
    public:
        menu_bar_page()
        {
            page_.set_title("Menu Bar");

            build_before_file_menu();
            build_file_menu();
            build_custom_menu();

            // ---- the visible body (the C# VerticalStackLayout Margin="12") ----
            menu_label_.set_text("You clicked on Menu Item:");
            body_.add(menu_label_);
            toggle_button_.set_text("Toggle Menu Bar Item");
            toggle_button_.clicked.connect([this]() { on_toggle_menu_bar_item(); });
            body_.add(toggle_button_);
            page_.set_content(body_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED VIEW, BOTTOM-UP, then re-host. The menu_bar_item / menu_flyout
        // item / sub_item / separator members are NON-view items (no standalone handler) and are excluded
        // — attaching one would throw (chrome_page.hpp convention). (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, menu_label_, "menu_label_");
            gallery_attach_one(app, toggle_button_, "toggle_button_");
            gallery_attach_one(app, body_, "body_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(body_); // the body hosts the label + toggle button
            gallery_rehost_content(page_);
        }

        // Owned controls / menus exposed for the hosting main / the headless test tree.
        [[nodiscard]] maui::controls::label& menu_label()
        {
            return menu_label_;
        }
        [[nodiscard]] maui::controls::button& toggle_button()
        {
            return toggle_button_;
        }
        [[nodiscard]] maui::controls::menu_bar_item& before_file_menu()
        {
            return before_file_menu_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& before_file_action()
        {
            return before_file_action_;
        }
        [[nodiscard]] maui::controls::menu_bar_item& custom_menu()
        {
            return custom_menu_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& custom_item1()
        {
            return custom_item1_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& sub_flyout_item1()
        {
            return sub_flyout_item1_;
        }
        // Whether the toggled-in "Added Menu" is currently in the bar (the C# FirstOrDefault present-ness).
        [[nodiscard]] bool added_menu_present() const
        {
            return added_menu_present_;
        }

    private:
        // "Before File": action (accel "b"), "Cool item 1", separator, "Cool item 2 (Disabled)".
        void build_before_file_menu()
        {
            before_file_menu_.set_text("Before File");

            wire_clicked(before_file_action_, "Before File Action");
            before_file_action_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::none, "b"});
            cool_item1_.set_text("Cool item 1"); // no Clicked in XAML — inert label-style item
            cool_item2_.set_text("Cool item 2 (Disabled)");
            cool_item2_.set_is_enabled(false);

            before_file_menu_.items().add(before_file_action_);
            before_file_menu_.items().add(cool_item1_);
            before_file_menu_.items().add(before_file_separator_);
            before_file_menu_.items().add(cool_item2_);
            page_.menu_bar_items().add(before_file_menu_);
        }

        // "File": "Custom File" with an Ionicons FontImageSource icon + Ctrl+Shift+F / Shift+F accelerators.
        void build_file_menu()
        {
            file_menu_.set_text("File");
            wire_clicked(custom_file_, "Custom File");
            // Ionicons glyph  (FontAutoScalingEnabled=False — built via a non-scaling font).
            custom_file_.set_icon_image_source(std::make_shared<maui::controls::font_image_source>(
                "\xEF\x8C\x8C",
                maui::core::font::of_size("Ionicons", maui::controls::font_image_source::default_size,
                                          maui::core::font_weight::regular, maui::core::font_slant::normal,
                                          /*enable_scaling=*/false),
                maui::graphics::colors::black));
            custom_file_.accelerators().push_back(
                {maui::core::keyboard_accelerator_modifiers::ctrl | maui::core::keyboard_accelerator_modifiers::shift,
                 "F"});
            custom_file_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::shift, "F"});
            file_menu_.items().add(custom_file_);
            page_.menu_bar_items().add(file_menu_);
        }

        // "Custom Menu": "Item 1" (Ctrl/Cmd+1), "Sub Menu 1" { mic / coffee flyout items }, "Item 2 (Disabled)".
        void build_custom_menu()
        {
            custom_menu_.set_text("Custom Menu");

            wire_clicked(custom_item1_, "Item 1");
            // OnPlatform WinUI=Ctrl / MacCatalyst=Cmd — resolved to ctrl here (gallery has no OnPlatform; note).
            custom_item1_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::ctrl, "1"});

            wire_clicked(sub_menu1_, "Sub Menu 1"); // the sub-item itself is clickable in XAML
            wire_clicked(sub_flyout_item1_, "Flyout item 1");
            sub_flyout_item1_.set_icon_image_source(std::make_shared<maui::controls::file_image_source>("mic.png"));
            sub_flyout_item1_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::windows |
                                                            maui::core::keyboard_accelerator_modifiers::shift,
                                                        "A"});
            wire_clicked(sub_flyout_item2_, "Flyout item 2");
            sub_flyout_item2_.set_icon_image_source(std::make_shared<maui::controls::file_image_source>("coffee.png"));
            sub_flyout_item2_.accelerators().push_back({maui::core::keyboard_accelerator_modifiers::alt, "C"});
            sub_menu1_.items().add(sub_flyout_item1_);
            sub_menu1_.items().add(sub_flyout_item2_);

            custom_item2_.set_text("Item 2 (Disabled)");
            custom_item2_.set_is_enabled(false);

            custom_menu_.items().add(custom_item1_);
            custom_menu_.items().add(sub_menu1_);
            custom_menu_.items().add(custom_item2_);
            page_.menu_bar_items().add(custom_menu_);
        }

        // The C# OnToggleMenuBarItem: add the owned "Added Menu" (with two flyout items, the second
        // disabled) when it is absent, remove it when present. The C# wires both added items' Command to
        // ItemClicked(barItem.First()) — i.e. clicking either echoes the FIRST item's text; the port maps
        // that command channel onto clicked (menu_item.hpp), connecting each to item_clicked(added_first_).
        void on_toggle_menu_bar_item()
        {
            if (added_menu_present_)
            {
                remove_menu_bar_item(added_menu_);
                added_menu_present_ = false;
                stamp_label("Removed Added Menu");
                return;
            }

            // Build the added menu lazily on first toggle-in (the items persist; re-adding re-uses them).
            if (added_first_.text().empty())
            {
                added_menu_.set_text("Added Menu");
                added_first_.set_text("Added Flyout Item");
                // C#: Command = ItemClicked(barItem.First()) → echoes the first item's text on click.
                added_first_.clicked.connect([this]() { item_clicked(added_first_); });
                added_second_.set_text("Added Disabled Flyout Item");
                added_second_.set_is_enabled(false);
                added_second_.clicked.connect([this]() { item_clicked(added_first_); });
                added_menu_.items().add(added_first_);
                added_menu_.items().add(added_second_);
            }

            page_.menu_bar_items().add(added_menu_);
            added_menu_present_ = true;
            stamp_label("Added Menu");
        }

        // Remove a menu_bar_item from the page's bar by scanning for its index (the list has no by-value
        // remove; find then remove_at — the menu_element_list API).
        void remove_menu_bar_item(maui::controls::menu_bar_item& target)
        {
            auto& bar = page_.menu_bar_items();
            for (std::size_t i = 0; i < bar.count(); ++i)
            {
                if (bar.at(i) == &target)
                {
                    bar.remove_at(i);
                    return;
                }
            }
        }

        // Wire a clickable flyout item: set its text and route its click through item_clicked (the C#
        // Clicked="ItemClicked").
        void wire_clicked(maui::controls::menu_flyout_item& item, const char* text)
        {
            item.set_text(text);
            item.clicked.connect([this, &item]() { item_clicked(item); });
        }

        // The C# ItemClicked: menuLabel.Text = "You clicked on Menu Item: <text>".
        void item_clicked(const maui::controls::menu_flyout_item& item)
        {
            menu_label_.set_text("You clicked on Menu Item: " + std::string(item.text()));
        }

        // A secondary status note on the visible label for the toggle action (keeps the toggle observable
        // even though it changes the bar, which the gallery doesn't mount).
        void stamp_label(const std::string& what)
        {
            menu_label_.set_text("Menu bar: " + what);
        }

        bool added_menu_present_ = false;

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout body_;
        maui::controls::label menu_label_;
        maui::controls::button toggle_button_;

        // ---- "Before File" menu ----
        maui::controls::menu_bar_item before_file_menu_;
        maui::controls::menu_flyout_item before_file_action_;
        maui::controls::menu_flyout_item cool_item1_;
        maui::controls::menu_flyout_separator before_file_separator_;
        maui::controls::menu_flyout_item cool_item2_;

        // ---- "File" menu ----
        maui::controls::menu_bar_item file_menu_;
        maui::controls::menu_flyout_item custom_file_;

        // ---- "Custom Menu" menu ----
        maui::controls::menu_bar_item custom_menu_;
        maui::controls::menu_flyout_item custom_item1_;
        maui::controls::menu_flyout_sub_item sub_menu1_;
        maui::controls::menu_flyout_item sub_flyout_item1_;
        maui::controls::menu_flyout_item sub_flyout_item2_;
        maui::controls::menu_flyout_item custom_item2_;

        // ---- the toggled-in "Added Menu" (OnToggleMenuBarItem) ----
        maui::controls::menu_bar_item added_menu_;
        maui::controls::menu_flyout_item added_first_;
        maui::controls::menu_flyout_item added_second_;
    };
} // namespace maui::samples
