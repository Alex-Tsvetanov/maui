#pragma once
// maui::samples::vertical_stack_layout_page — ports VerticalStackLayoutPage.xaml
//
// Demonstrates maui::controls::vertical_stack_layout (the fixed top-to-bottom stack) by holding a label
// followed by the same six colored box_views (Red/Yellow/Blue/Green/Orange/Purple), stacked top-to-
// bottom, with a 12px margin. Mirrors the XAML: a VerticalStackLayout (Margin=12) with a "VerticalStack
// Layout" label and the six boxes. (XAML Margin maps to the stack's padding here — the closest
// headless-safe analogue.)
//
// The page OWNS its whole element tree (the sample_app pattern). attach_handlers attaches a handler to
// every owned VIEW bottom-up via a GENERIC lambda that preserves each member's concrete static type,
// then replays the host "add"/"set_content" commands so the native panel mirrors the children built in
// the ctor.

#include <cstdio>
#include <exception>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class vertical_stack_layout_page
    {
    public:
        vertical_stack_layout_page()
        {
            page_.set_title("VerticalStackLayout");

            // VerticalStackLayout (Margin=12 → padding here, the headless-safe analogue).
            stack_.set_padding(maui::core::thickness(12));

            heading_.set_text("VerticalStackLayout");

            red_.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));      // Red
            yellow_.set_color(maui::graphics::color(1.0F, 1.0F, 0.0F));   // Yellow
            blue_.set_color(maui::graphics::color(0.0F, 0.0F, 1.0F));     // Blue
            green_.set_color(maui::graphics::color(0.0F, 0.50F, 0.0F));   // Green
            orange_.set_color(maui::graphics::color(1.0F, 0.65F, 0.0F));  // Orange
            purple_.set_color(maui::graphics::color(0.50F, 0.0F, 0.50F)); // Purple

            stack_.add(heading_);
            stack_.add(red_);
            stack_.add(yellow_);
            stack_.add(blue_);
            stack_.add(green_);
            stack_.add(orange_);
            stack_.add(purple_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaf label/boxes first, the stack, the page
        // last), then replay the host commands so the native panel mirrors the ctor-built tree. A GENERIC
        // lambda preserves each member's concrete static type — never use i_view& (it erases the type and
        // attach_handler finds no handler → blank page).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& v, const char* n) {
                try
                {
                    app.attach_handler(v);
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", n, e.what());
                }
            };

            one(heading_, "heading_");
            one(red_, "red_");
            one(yellow_, "yellow_");
            one(blue_, "blue_");
            one(green_, "green_");
            one(orange_, "orange_");
            one(purple_, "purple_");
            one(stack_, "stack_");
            one(page_, "page_");

            if (const auto& stack_handler = stack_.handler())
            {
                for (int i = 0; i < stack_.count(); ++i)
                {
                    stack_handler->invoke("add", maui::core::layout_handler_update{.index = i, .view = &stack_.at(i)});
                }
            }
            if (const auto& page_handler = page_.handler())
            {
                page_handler->invoke("set_content");
            }
        }

        // ---- owned controls, exposed for the hosting main's bottom-up attachment ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& heading()
        {
            return heading_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label heading_;
        maui::controls::box_view red_;
        maui::controls::box_view yellow_;
        maui::controls::box_view blue_;
        maui::controls::box_view green_;
        maui::controls::box_view orange_;
        maui::controls::box_view purple_;
    };
} // namespace maui::samples
