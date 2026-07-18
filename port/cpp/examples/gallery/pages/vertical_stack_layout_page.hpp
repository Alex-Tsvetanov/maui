#pragma once
// maui::samples::vertical_stack_layout_page — ports VerticalStackLayoutPage.xaml
//
// Demonstrates maui::controls::vertical_stack_layout (the fixed top-to-bottom stack) by holding a label
// followed by the same six colored box_views (Red/Yellow/Blue/Green/Orange/Purple), stacked top-to-
// bottom, with a 12px margin. Mirrors the XAML: a VerticalStackLayout (Margin=12) with a "VerticalStack
// Layout" label and the six boxes. (XAML Margin maps to the stack's padding here — the closest
// headless-safe analogue.)
//
// The page OWNS its whole element tree (the sample_app pattern). The generic mount attaches a handler to
// every owned VIEW bottom-up via a GENERIC lambda that preserves each member's concrete static type,
// then replays the host "add"/"set_content" commands so the native panel mirrors the children built in
// the ctor.

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

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
            stack_.set_spacing(6); // shared XAML <VerticalStackLayout … Spacing="6"> (was missing → rows drifted)

            heading_.set_text("VerticalStackLayout");

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
