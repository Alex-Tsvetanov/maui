#pragma once
// maui::samples::horizontal_stack_layout_page — ports HorizontalStackLayoutPage.xaml
//
// Demonstrates maui::controls::horizontal_stack_layout (the fixed left-to-right stack): the six colored
// box_views (Red/Yellow/Blue/Green/Orange/Purple), stacked left-to-right, in a HorizontalStackLayout with
// Padding=12 and Spacing=6 — an EXACT mirror of the shared horizontal_stack.xaml (no heading label; an
// earlier twin added one, which pushed the boxes right and drew extra title text vs MAUI).
//
// The page OWNS its whole element tree (the sample_app pattern). The generic mount (app_host.hpp)
// attaches a handler to every owned VIEW bottom-up, preserving each member's concrete static type,
// then replays the host "add"/"set_content" commands so the native panel mirrors the children built in
// the ctor.

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class horizontal_stack_layout_page
    {
    public:
        horizontal_stack_layout_page()
        {
            page_.set_title("HorizontalStackLayout");

            // HorizontalStackLayout Padding="12" Spacing="6" (the shared XAML exactly).
            stack_.set_padding(maui::core::thickness(12));
            stack_.set_spacing(6);

            red_.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));      // Red
            yellow_.set_color(maui::graphics::color(1.0F, 1.0F, 0.0F));   // Yellow
            blue_.set_color(maui::graphics::color(0.0F, 0.0F, 1.0F));     // Blue
            green_.set_color(maui::graphics::color(0.0F, 0.50F, 0.0F));   // Green
            orange_.set_color(maui::graphics::color(1.0F, 0.65F, 0.0F));  // Orange
            purple_.set_color(maui::graphics::color(0.50F, 0.0F, 0.50F)); // Purple

            // Give every BoxView an explicit 40x40 size: a modern MAUI BoxView with no size request
            // measures to 0 and collapses (the maui-compare reference does the same), so both sides set the
            // SAME size to keep the demo visible AND matching (the harness-parity choice).
            for (auto* box : {&red_, &yellow_, &blue_, &green_, &orange_, &purple_})
            {
                box->set_width_request(40);
                box->set_height_request(40);
            }

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

        // ---- owned controls, exposed for the hosting main's bottom-up attachment ----
        [[nodiscard]] maui::controls::horizontal_stack_layout& stack()
        {
            return stack_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::horizontal_stack_layout stack_;
        maui::controls::box_view red_;
        maui::controls::box_view yellow_;
        maui::controls::box_view blue_;
        maui::controls::box_view green_;
        maui::controls::box_view orange_;
        maui::controls::box_view purple_;
    };
} // namespace maui::samples
