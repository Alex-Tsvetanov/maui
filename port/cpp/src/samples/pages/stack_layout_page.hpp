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
// The page OWNS its whole element tree (the sample_app pattern). attach_handlers attaches a handler to
// every owned VIEW bottom-up via a GENERIC lambda that preserves each member's concrete static type
// (attach_handler keys on it), then replays the host "add"/"set_content" commands so the native panels
// actually mirror the children built in the ctor.

#include <cstdio>
#include <exception>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stack_orientation.hpp"
#include "maui/core/font.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/hosting/maui_app.hpp"

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

        // Attach a handler to every OWNED view, BOTTOM-UP (leaf boxes/labels first, inner stacks, the
        // outer stack, the page last), then replay the host commands so the native panels mirror the tree
        // built in the ctor. A GENERIC lambda preserves each member's concrete static type — never use
        // i_view& here (that erases the type and attach_handler finds no handler → blank page).
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

            one(vertical_heading_, "vertical_heading_");
            one(vertical_red_, "vertical_red_");
            one(vertical_yellow_, "vertical_yellow_");
            one(vertical_blue_, "vertical_blue_");
            one(vertical_green_, "vertical_green_");
            one(vertical_orange_, "vertical_orange_");
            one(vertical_purple_, "vertical_purple_");
            one(vertical_stack_, "vertical_stack_");
            one(horizontal_heading_, "horizontal_heading_");
            one(horizontal_red_, "horizontal_red_");
            one(horizontal_yellow_, "horizontal_yellow_");
            one(horizontal_blue_, "horizontal_blue_");
            one(horizontal_green_, "horizontal_green_");
            one(horizontal_orange_, "horizontal_orange_");
            one(horizontal_purple_, "horizontal_purple_");
            one(horizontal_stack_, "horizontal_stack_");
            one(outer_, "outer_");
            one(page_, "page_");

            // Replay the layout "add" commands (children added in the ctor, before handlers existed) and
            // the page's "set_content", so the native subview trees actually materialize.
            rehost_layout(vertical_stack_);
            rehost_layout(horizontal_stack_);
            rehost_layout(outer_);
            if (const auto& h = page_.handler())
            {
                h->invoke("set_content");
            }
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
        // Replay a layout's children into its now-attached handler so the native panel hosts each one.
        template <class Layout> static void rehost_layout(Layout& layout)
        {
            if (const auto& layout_handler = layout.handler())
            {
                for (int i = 0; i < layout.count(); ++i)
                {
                    layout_handler->invoke("add", maui::core::layout_handler_update{.index = i, .view = &layout.at(i)});
                }
            }
        }

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
