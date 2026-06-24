#pragma once
// maui::samples::stack_layout_page — ports StackLayoutPage.xaml
//
// Demonstrates the generic maui::controls::stack_layout (the orientation-switching sibling of the
// fixed vertical/horizontal stacks) by nesting two inner stacks inside an outer vertical stack with a
// 12px margin: a "Vertical" section whose inner stack_layout keeps the default vertical orientation,
// and a "Horizontal" section whose inner stack_layout is flipped to stack_orientation::horizontal —
// each filled with the same six colored box_views (Red/Yellow/Blue/Green/Orange/Purple), so the
// orientation difference is visible. Mirrors the XAML: an outer StackLayout (Margin=12) holding two
// Headline labels and two inner StackLayouts. (XAML Margin maps to the outer stack's padding here —
// the closest headless-safe analogue; note: the C# Headline StaticResource is approximated with a bold
// system font on the section labels.)
//
// The page OWNS its whole element tree (the sample_app pattern).

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::samples
{
    class stack_layout_page
    {
    public:
        stack_layout_page()
        {
            page_.set_title("StackLayout");

            // Outer StackLayout (Margin=12 → padding here, the headless-safe analogue).
            outer_.set_padding(maui::core::thickness(12));

            // Headline section labels (Headline StaticResource → bold system font, best-effort).
            vertical_heading_.set_text("Vertical");
            vertical_heading_.set_font(maui::core::font::system_font_of_size(20, maui::core::font_weight::bold));
            horizontal_heading_.set_text("Horizontal");
            horizontal_heading_.set_font(maui::core::font::system_font_of_size(20, maui::core::font_weight::bold));

            // Vertical inner stack — default orientation (vertical), six colored boxes.
            vertical_red_.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));      // Red
            vertical_yellow_.set_color(maui::graphics::color(1.0F, 1.0F, 0.0F));   // Yellow
            vertical_blue_.set_color(maui::graphics::color(0.0F, 0.0F, 1.0F));     // Blue
            vertical_green_.set_color(maui::graphics::color(0.0F, 0.50F, 0.0F));   // Green
            vertical_orange_.set_color(maui::graphics::color(1.0F, 0.65F, 0.0F));  // Orange
            vertical_purple_.set_color(maui::graphics::color(0.50F, 0.0F, 0.50F)); // Purple
            vertical_stack_.add(vertical_red_);
            vertical_stack_.add(vertical_yellow_);
            vertical_stack_.add(vertical_blue_);
            vertical_stack_.add(vertical_green_);
            vertical_stack_.add(vertical_orange_);
            vertical_stack_.add(vertical_purple_);

            // Horizontal inner stack — Orientation flipped to horizontal, same six boxes.
            horizontal_stack_.set_orientation(maui::controls::stack_orientation::horizontal);
            horizontal_red_.set_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
            horizontal_yellow_.set_color(maui::graphics::color(1.0F, 1.0F, 0.0F));
            horizontal_blue_.set_color(maui::graphics::color(0.0F, 0.0F, 1.0F));
            horizontal_green_.set_color(maui::graphics::color(0.0F, 0.50F, 0.0F));
            horizontal_orange_.set_color(maui::graphics::color(1.0F, 0.65F, 0.0F));
            horizontal_purple_.set_color(maui::graphics::color(0.50F, 0.0F, 0.50F));

            // Give every BoxView an explicit 40x40 size: a modern MAUI BoxView with no size request
            // measures to 0 and collapses (the maui-compare reference does the same), so both sides set the
            // SAME size to keep the demo visible AND matching (the harness-parity choice).
            for (auto* box : {&vertical_red_, &vertical_yellow_, &vertical_blue_, &vertical_green_, &vertical_orange_,
                              &vertical_purple_, &horizontal_red_, &horizontal_yellow_, &horizontal_blue_,
                              &horizontal_green_, &horizontal_orange_, &horizontal_purple_})
            {
                box->set_width_request(40);
                box->set_height_request(40);
            }

            horizontal_stack_.add(horizontal_red_);
            horizontal_stack_.add(horizontal_yellow_);
            horizontal_stack_.add(horizontal_blue_);
            horizontal_stack_.add(horizontal_green_);
            horizontal_stack_.add(horizontal_orange_);
            horizontal_stack_.add(horizontal_purple_);

            outer_.add(vertical_heading_);
            outer_.add(vertical_stack_);
            outer_.add(horizontal_heading_);
            outer_.add(horizontal_stack_);
            page_.set_content(outer_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls, exposed for the hosting main's bottom-up attachment ----
        [[nodiscard]] maui::controls::stack_layout& outer()
        {
            return outer_;
        }
        [[nodiscard]] maui::controls::stack_layout& vertical_stack()
        {
            return vertical_stack_;
        }
        [[nodiscard]] maui::controls::stack_layout& horizontal_stack()
        {
            return horizontal_stack_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::stack_layout outer_;

        maui::controls::label vertical_heading_;
        maui::controls::stack_layout vertical_stack_;
        maui::controls::box_view vertical_red_;
        maui::controls::box_view vertical_yellow_;
        maui::controls::box_view vertical_blue_;
        maui::controls::box_view vertical_green_;
        maui::controls::box_view vertical_orange_;
        maui::controls::box_view vertical_purple_;

        maui::controls::label horizontal_heading_;
        maui::controls::stack_layout horizontal_stack_;
        maui::controls::box_view horizontal_red_;
        maui::controls::box_view horizontal_yellow_;
        maui::controls::box_view horizontal_blue_;
        maui::controls::box_view horizontal_green_;
        maui::controls::box_view horizontal_orange_;
        maui::controls::box_view horizontal_purple_;
    };
} // namespace maui::samples
