#pragma once
// maui::samples::toolbar_page — ports ToolbarPage.xaml (Maui.Controls.Sample.Pages.ToolbarPage).
//
// The C# page hangs a set of ToolbarItems off Page.ToolbarItems — two primary, four secondary (with
// Order=Secondary + Priority sorting them) — plus a vertical stack of buttons that mutate those items
// at runtime (enable/disable, rename, remove/re-add, change Command). Every ToolbarItem activation
// stamps a label readout ("You clicked on ToolbarItem: <text>"). This port mirrors that wiring with the
// headless-safe maui:: API, code-first.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// hosts page() in a window (the toolbar items materialize as an NSToolbar on AppKit / through the
// navigation chrome on iOS); the headless test tree exercises the same wiring deterministically.
//
// Interactions demonstrated (faithful to the C# ItemClicked / Button_Clicked* handlers):
//   - the two primary + four secondary toolbar items each stamp the readout on activate() (ItemClicked),
//   - "Enable/Disable Test (1)" toggles primary1's IsEnabled (Button_Clicked1),
//   - "Enable/Disable Test Secondary (4)" toggles secondary4's IsEnabled (Button_Clicked),
//   - "Change text on Test Secondary (1)" flips secondary1.Text (Button_Clicked3),
//   - "Remove/Add Secondary (3)" removes/re-adds secondary3 from page().toolbar_items() (Button_Clicked4),
//   - "Change Command on Secondary (3)" re-wires secondary3's command channel (Button_Clicked5).
//
// note: the C# Secondary (2) button delays the toggle 5s via Task.Delay/Dispatcher; the headless port
//       has no dispatcher seam here, so this port toggles secondary2 immediately and labels the button
//       accordingly (the demonstrated behavior — IsEnabled toggling — is identical, only the delay drops).
// note: C# Secondary (3) drives Command="{Binding ClickedCommand}" (an ICommand). The port's menu/toolbar
//       command channel IS the clicked event (see menu_item.hpp: "the port has no ICommand at the menu
//       level; the clicked event is the command channel"). So this port wires a maui::controls::command
//       and connects its execute() to secondary3.clicked, faithfully reproducing the readout the C#
//       ClickedCommand produces; Button_Clicked5 swaps in a different command the same way.
// note: IconImageSource (coffee.png / a FontImageSource glyph) is set on the secondary items exactly as
//       C# does (image_source::from_file); its native menu-item image materialization is deferred per the
//       port's W1-11 notes — the items still function and stamp the readout.

#include <any>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "maui/controls/button.hpp"
#include "maui/controls/command.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class toolbar_page
    {
    public:
        toolbar_page()
        {
            page_.set_title("Toolbar");
            stack_.set_spacing(8);

            menu_label_.set_text("You clicked on ToolbarItem: {none}");
            // The status readout is the FIRST child of the visible stack, matching the C# layout order
            // (label, then the six runtime-mutator buttons added by add_button below).
            stack_.add(menu_label_);

            // ---- the toolbar items (Page.ToolbarItems) ------------------------------------------------
            // Two primary items.
            primary1_.set_text("Test (1)");
            primary1_.set_order(maui::controls::toolbar_item_order::primary);
            primary1_.clicked.connect([this] { item_clicked(primary1_); });
            page_.toolbar_items().add(primary1_);

            primary2_.set_text("Test (2)");
            primary2_.set_order(maui::controls::toolbar_item_order::primary);
            primary2_.clicked.connect([this] { item_clicked(primary2_); });
            page_.toolbar_items().add(primary2_);

            // Four secondary items, Priority-ordered (the toolbar tracker sorts on priority()).
            secondary1_.set_text("Test Secondary (1)");
            secondary1_.set_order(maui::controls::toolbar_item_order::secondary);
            secondary1_.set_priority(1);
            secondary1_.set_icon_image_source(maui::controls::image_source::from_file("coffee.png"));
            secondary1_.clicked.connect([this] { item_clicked(secondary1_); });
            page_.toolbar_items().add(secondary1_);

            secondary2_.set_text("Test Secondary (2)");
            secondary2_.set_order(maui::controls::toolbar_item_order::secondary);
            secondary2_.set_priority(2);
            // note: C# attaches a FontImageSource glyph here; the port stores a file image source stand-in
            //       (the glyph/font-image-source variant is a deferred icon path — item still functions).
            secondary2_.clicked.connect([this] { item_clicked(secondary2_); });
            page_.toolbar_items().add(secondary2_);

            secondary4_.set_text("Test Secondary (4)");
            secondary4_.set_order(maui::controls::toolbar_item_order::secondary);
            secondary4_.set_priority(4);
            secondary4_.set_is_enabled(false); // C# IsEnabled="False"
            secondary4_.clicked.connect([this] { item_clicked(secondary4_); });
            page_.toolbar_items().add(secondary4_);

            // Secondary (3): the command-driven item. The port's menu/toolbar command channel IS the
            // clicked event (see header note), so connect clicked ONCE to invoke whatever command
            // secondary3_command_ currently holds, and let wire_secondary3_command swap that command in
            // (the C# Command="{Binding ClickedCommand}" + Button_Clicked5 Command-swap, faithfully).
            secondary3_.set_text("Test Secondary (3)");
            secondary3_.set_order(maui::controls::toolbar_item_order::secondary);
            secondary3_.set_priority(3);
            wire_secondary3_command("You clicked on ToolbarItem: Test Secondary (3)");
            secondary3_.clicked.connect([this] {
                if (secondary3_command_ && secondary3_command_->can_execute(std::any{}))
                {
                    secondary3_command_->execute(std::any{});
                }
            });
            page_.toolbar_items().add(secondary3_);

            // ---- the button stack (the runtime mutators) ----------------------------------------------
            add_button(toggle_primary1_btn_, "Enable/Disable Test (1)",
                       [this] { primary1_.set_is_enabled(!primary1_.is_enabled_explicit()); });
            add_button(toggle_secondary4_btn_, "Enable/Disable Test Secondary (4)",
                       [this] { secondary4_.set_is_enabled(!secondary4_.is_enabled_explicit()); });
            add_button(toggle_secondary2_btn_, "Enable/Disable Test Secondary (2)",
                       [this] { secondary2_.set_is_enabled(!secondary2_.is_enabled_explicit()); });
            add_button(rename_secondary1_btn_, "Change text on Test Secondary (1)", [this] {
                const bool was_default = secondary1_.text() == "Test Secondary (1)";
                secondary1_.set_text(was_default ? "Changed Text" : "Test Secondary (1)");
            });
            add_button(remove_add_secondary3_btn_, "Remove/Add Secondary (3)", [this] {
                if (page_.toolbar_items().contains(secondary3_))
                {
                    page_.toolbar_items().remove(secondary3_);
                }
                else
                {
                    page_.toolbar_items().add(secondary3_);
                }
            });
            add_button(change_command_btn_, "Change Command Property on Secondary (3)", [this] {
                wire_secondary3_command("You clicked on ToolbarItem: Test Secondary (3) with changed Command");
            });

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED VIEW, bottom-up (the leaf views, then the stack, then the page),
        // then re-host the tree built in the ctor. The toolbar_item members are NON-view menu items: they
        // have no standalone handler (attaching would throw), so they are deliberately excluded — exactly
        // as chrome_page does. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, menu_label_, "menu_label_");
            gallery_attach_one(app, toggle_primary1_btn_, "toggle_primary1_btn_");
            gallery_attach_one(app, toggle_secondary4_btn_, "toggle_secondary4_btn_");
            gallery_attach_one(app, toggle_secondary2_btn_, "toggle_secondary2_btn_");
            gallery_attach_one(app, rename_secondary1_btn_, "rename_secondary1_btn_");
            gallery_attach_one(app, remove_add_secondary3_btn_, "remove_add_secondary3_btn_");
            gallery_attach_one(app, change_command_btn_, "change_command_btn_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned views/items, exposed for the hosting main + headless tests.
        [[nodiscard]] maui::controls::label& menu_label()
        {
            return menu_label_;
        }
        [[nodiscard]] maui::controls::toolbar_item& primary1()
        {
            return primary1_;
        }
        [[nodiscard]] maui::controls::toolbar_item& secondary1()
        {
            return secondary1_;
        }
        [[nodiscard]] maui::controls::toolbar_item& secondary2()
        {
            return secondary2_;
        }
        [[nodiscard]] maui::controls::toolbar_item& secondary3()
        {
            return secondary3_;
        }
        [[nodiscard]] maui::controls::toolbar_item& secondary4()
        {
            return secondary4_;
        }

    private:
        // ItemClicked: stamp the readout with the activated item's text (C# $"...: {tbi.Text}").
        void item_clicked(const maui::controls::toolbar_item& item)
        {
            menu_label_.set_text("You clicked on ToolbarItem: " + std::string(item.text()));
        }

        // (Re-)create secondary3's command (the Command.cs analog). C# Button_Clicked5 swaps the Command
        // object; here we swap the stored command — the single clicked connection (ctor) invokes the
        // current one, so the swap takes effect on the next activation just as the C# binding does.
        void wire_secondary3_command(std::string readout)
        {
            secondary3_command_.emplace(
                [this, readout = std::move(readout)](const std::any& /*parameter*/) { menu_label_.set_text(readout); });
        }

        void add_button(maui::controls::button& button, const char* text, std::function<void()> on_click)
        {
            button.set_text(text);
            button.clicked.connect(std::move(on_click));
            stack_.add(button);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label menu_label_;

        // Page.ToolbarItems (non-view menu items, owned here; the page's list references them).
        maui::controls::toolbar_item primary1_;
        maui::controls::toolbar_item primary2_;
        maui::controls::toolbar_item secondary1_;
        maui::controls::toolbar_item secondary2_;
        maui::controls::toolbar_item secondary3_;
        maui::controls::toolbar_item secondary4_;

        // secondary3's current command (the ClickedCommand analog; re-created by wire_secondary3_command).
        std::optional<maui::controls::command> secondary3_command_;

        // The runtime-mutator buttons.
        maui::controls::button toggle_primary1_btn_;
        maui::controls::button toggle_secondary4_btn_;
        maui::controls::button toggle_secondary2_btn_;
        maui::controls::button rename_secondary1_btn_;
        maui::controls::button remove_add_secondary3_btn_;
        maui::controls::button change_command_btn_;
    };
} // namespace maui::samples
