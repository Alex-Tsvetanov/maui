#pragma once
// chrome_page — a self-contained demo page for the W1-11 window-chrome family: page toolbar items
// (primary + secondary), a menu bar (File menu with items, a separator and a sub-menu), a context
// flyout (right-click menu) on a button, and a tooltip — all wired so every activation drives a
// visible label readout (the C# gallery-page convention, code-first).
//
// The page OWNS its whole element tree (the sample_app pattern in maui_app_sample.mm). It is
// backend-agnostic — a sample main hosts page() in a window (the window chrome then materializes the
// NSToolbar / NSMenu main menu on AppKit; on iOS the toolbar items surface through the navigation
// chrome and menus stay stored-inert, C# parity); the headless test tree exercises the same wiring
// deterministically (chrome_page_tests.cpp).
//
// Interactions demonstrated:
//   - the "Save" (primary) and "About" (secondary/overflow) toolbar items stamp the readout,
//   - the File menu's "New" / "Open" flyout items and the "Recent > First" sub-menu item stamp it too,
//   - the button's context flyout offers Copy/Paste around a separator,
//   - the button carries a tooltip ("Press or right-click me").

#include <cstdio>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/menu_bar_item.hpp"
#include "maui/controls/menu_flyout.hpp"
#include "maui/controls/menu_flyout_item.hpp"
#include "maui/controls/menu_flyout_separator.hpp"
#include "maui/controls/menu_flyout_sub_item.hpp"
#include "maui/controls/tool_tip_properties.hpp"
#include "maui/controls/toolbar_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class chrome_page
    {
    public:
        chrome_page()
        {
            page_.set_title("Chrome");
            stack_.set_spacing(12);
            readout_.set_text("Ready");

            // ---- toolbar items (Page.ToolbarItems): one primary, one secondary (overflow) ----
            save_item_.set_text("Save");
            save_item_.set_order(maui::controls::toolbar_item_order::primary);
            save_item_.clicked.connect([this] { stamp("Saved"); });
            page_.toolbar_items().add(save_item_);

            about_item_.set_text("About");
            about_item_.set_order(maui::controls::toolbar_item_order::secondary);
            about_item_.clicked.connect([this] { stamp("About"); });
            page_.toolbar_items().add(about_item_);

            // ---- menu bar (Page.MenuBarItems): File > New / Open / ── / Recent > First ----
            file_menu_.set_text("File");
            new_item_.set_text("New");
            new_item_.clicked.connect([this] { stamp("File > New"); });
            open_item_.set_text("Open");
            open_item_.clicked.connect([this] { stamp("File > Open"); });
            recent_menu_.set_text("Recent");
            recent_first_.set_text("First");
            recent_first_.clicked.connect([this] { stamp("File > Recent > First"); });
            recent_menu_.items().add(recent_first_);
            file_menu_.items().add(new_item_);
            file_menu_.items().add(open_item_);
            file_menu_.items().add(separator_);
            file_menu_.items().add(recent_menu_);
            page_.menu_bar_items().add(file_menu_);

            // ---- the button: a tooltip + a context flyout (right-click menu) ----
            action_button_.set_text("Press or right-click me");
            action_button_.clicked.connect([this] { stamp("Button pressed"); });
            maui::controls::tool_tip_properties::set_text(action_button_, "Press or right-click me");
            copy_item_.set_text("Copy");
            copy_item_.clicked.connect([this] { stamp("Copy"); });
            paste_item_.set_text("Paste");
            paste_item_.clicked.connect([this] { stamp("Paste"); });
            context_menu_.items().add(copy_item_);
            context_menu_.items().add(context_separator_);
            context_menu_.items().add(paste_item_);
            action_button_.set_context_flyout(&context_menu_);

            stack_.add(action_button_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the button + readout, then the stack, then the
        // page), then re-host the tree built in the ctor. The toolbar_item / menu_bar_item / menu_flyout*
        // members are NON-view items: they have no standalone handler, so they are deliberately excluded
        // (attaching them would throw). (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, action_button_, "action_button_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::toolbar_item& save_item()
        {
            return save_item_;
        }
        [[nodiscard]] maui::controls::toolbar_item& about_item()
        {
            return about_item_;
        }
        [[nodiscard]] maui::controls::menu_bar_item& file_menu()
        {
            return file_menu_;
        }
        [[nodiscard]] maui::controls::menu_flyout& context_menu()
        {
            return context_menu_;
        }
        [[nodiscard]] maui::controls::menu_flyout_item& recent_first()
        {
            return recent_first_;
        }
        [[nodiscard]] maui::controls::button& action_button()
        {
            return action_button_;
        }

    private:
        void stamp(const std::string& what)
        {
            readout_.set_text("Last: " + what);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::button action_button_;
        maui::controls::label readout_;
        maui::controls::toolbar_item save_item_;
        maui::controls::toolbar_item about_item_;
        maui::controls::menu_bar_item file_menu_;
        maui::controls::menu_flyout_item new_item_;
        maui::controls::menu_flyout_item open_item_;
        maui::controls::menu_flyout_separator separator_;
        maui::controls::menu_flyout_sub_item recent_menu_;
        maui::controls::menu_flyout_item recent_first_;
        maui::controls::menu_flyout context_menu_;
        maui::controls::menu_flyout_item copy_item_;
        maui::controls::menu_flyout_separator context_separator_;
        maui::controls::menu_flyout_item paste_item_;
    };
} // namespace maui::samples
