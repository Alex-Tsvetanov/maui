#pragma once
// maui::samples::check_box_page — ports CheckBoxPage.xaml (+ .xaml.cs)
//
// Mirrors the MAUI gallery page: a vertical stack of headlined CheckBox states — Default, Colored
// (Color=Purple), Disabled, Disabled+Colored+Checked — followed by a "Change IsChecked" row pairing a
// Button with a CheckBox. The .xaml.cs behavior is preserved: clicking the button flips a green/red flag
// that recolors the paired check box (and the button's own text + text color), as UpdateControls() does.
//
// Self-contained (the value_controls_page pattern): the page OWNS its whole element tree as members,
// exposes page() and attach_handlers(maui_app). Headless-safe — only cross-platform maui:: API here.

#include <cstdio>

#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class check_box_page
    {
    public:
        check_box_page()
        {
            page_.set_title("CheckBox");
            stack_.set_spacing(6);

            // --- Default ---
            default_headline_.set_text("Default");

            // --- Colored (Color=Purple) ---
            colored_headline_.set_text("Colored");
            colored_check_.set_color(maui::graphics::colors::purple);

            // --- Disabled ---
            disabled_headline_.set_text("Disabled");
            disabled_check_.set_is_enabled(false);

            // --- Disabled Colored (IsEnabled=False, Color=Purple, IsChecked=True) ---
            disabled_colored_headline_.set_text("Disabled Colored");
            disabled_colored_check_.set_is_enabled(false);
            disabled_colored_check_.set_color(maui::graphics::colors::purple);
            disabled_colored_check_.set_is_checked(true);

            // --- Change IsChecked (Button + CheckBox row) ---
            change_headline_.set_text("Change IsChecked");
            change_check_.set_is_checked(true);
            // OnChangeIsCheckedButtonClicked: flip the green/red flag, then UpdateControls().
            change_button_.command = [this] {
                is_green_ = !is_green_;
                update_controls();
            };
            change_row_.add(change_button_);
            change_row_.add(change_check_);

            stack_.add(default_headline_);
            stack_.add(default_check_);
            stack_.add(colored_headline_);
            stack_.add(colored_check_);
            stack_.add(disabled_headline_);
            stack_.add(disabled_check_);
            stack_.add(disabled_colored_headline_);
            stack_.add(disabled_colored_check_);
            stack_.add(change_headline_);
            stack_.add(change_row_);
            page_.set_content(stack_);

            // Seed the button text/color the way the ctor's UpdateControls() call does.
            update_controls();
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view BOTTOM-UP (leaves → layouts → page), then replay the host
        // commands the ctor fired before any handler existed (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, default_headline_, "default_headline_");
            gallery_attach_one(app, default_check_, "default_check_");
            gallery_attach_one(app, colored_headline_, "colored_headline_");
            gallery_attach_one(app, colored_check_, "colored_check_");
            gallery_attach_one(app, disabled_headline_, "disabled_headline_");
            gallery_attach_one(app, disabled_check_, "disabled_check_");
            gallery_attach_one(app, disabled_colored_headline_, "disabled_colored_headline_");
            gallery_attach_one(app, disabled_colored_check_, "disabled_colored_check_");
            gallery_attach_one(app, change_headline_, "change_headline_");
            gallery_attach_one(app, change_button_, "change_button_");
            gallery_attach_one(app, change_check_, "change_check_");
            gallery_attach_one(app, change_row_, "change_row_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(change_row_);
            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

    private:
        // CheckBoxPage.UpdateControls(): recolor the paired check box + the button's text/color from the flag.
        void update_controls()
        {
            const auto accent = is_green_ ? maui::graphics::colors::green : maui::graphics::colors::red;
            change_check_.set_color(accent);
            change_button_.set_text_color(accent);
            char text[24];
            std::snprintf(text, sizeof(text), "Is green? %s", is_green_ ? "True" : "False");
            change_button_.set_text(text);
        }

        bool is_green_ = false; // CheckBoxPage._isGreen

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;

        maui::controls::label default_headline_;
        maui::controls::check_box default_check_;
        maui::controls::label colored_headline_;
        maui::controls::check_box colored_check_;
        maui::controls::label disabled_headline_;
        maui::controls::check_box disabled_check_;
        maui::controls::label disabled_colored_headline_;
        maui::controls::check_box disabled_colored_check_;
        maui::controls::label change_headline_;
        maui::controls::horizontal_stack_layout change_row_;
        maui::controls::button change_button_;
        maui::controls::check_box change_check_;
    };
} // namespace maui::samples
