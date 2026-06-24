#pragma once
// maui::samples::fonts_page — a faithful reproduction of the maui-compare "fonts" demo
// (ComparePages.Fonts()), the shipped-.NET-MAUI reference for the visual-parity comparison: a ScrollView
// over a VerticalStackLayout (Spacing 8, Padding 16) of nine Labels — Title/Subtitle/Header/Body/Caption
// at explicit decreasing FontSizes (34/28/23/22/18), then Bold / Italic / Bold+Italic via FontAttributes
// (default size), then a "Character spacing 4.0" label (CharacterSpacing 4.0). Kept 1:1 with the C#
// reference (same text, sizes, attributes, order).
//
// The port folds FontAttributes into the font's weight + slant (FontExtensions.WithAttributes): Bold →
// font_weight::bold, Italic → font_slant::italic. The attribute + character-spacing labels carry NO
// explicit size, matching the C# (system default), via font::system_font_of_weight.
//
// The page OWNS its whole tree; the generic mount attaches every owned view's handler bottom-up. ScrollView wraps the
// stack (a single-content host).

#include <array>
#include <cstddef>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"

namespace maui::samples
{
    class fonts_page
    {
    public:
        fonts_page()
        {
            page_.set_title("Fonts");
            stack_.set_spacing(8);
            stack_.set_padding(maui::core::thickness(16));

            // Title/Subtitle/Header/Body/Caption at explicit FontSizes.
            title_.set_text("Title");
            title_.set_font(maui::core::font::system_font_of_size(34));
            subtitle_.set_text("Subtitle");
            subtitle_.set_font(maui::core::font::system_font_of_size(28));
            header_.set_text("Header");
            header_.set_font(maui::core::font::system_font_of_size(23));
            body_.set_text("Body");
            body_.set_font(maui::core::font::system_font_of_size(22));
            caption_.set_text("Caption");
            caption_.set_font(maui::core::font::system_font_of_size(18));

            // FontAttributes Bold / Italic / Bold|Italic at the default size.
            bold_.set_text("Bold");
            bold_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));
            italic_.set_text("Italic");
            italic_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::regular,
                                                                     maui::core::font_slant::italic));
            bold_italic_.set_text("Bold + Italic");
            bold_italic_.set_font(
                maui::core::font::system_font_of_weight(maui::core::font_weight::bold, maui::core::font_slant::italic));

            // CharacterSpacing 4.0 (no explicit size — the default font).
            kerned_.set_text("Character spacing 4.0");
            kerned_.set_character_spacing(4.0);

            for (auto* label :
                 {&title_, &subtitle_, &header_, &body_, &caption_, &bold_, &italic_, &bold_italic_, &kerned_})
            {
                stack_.add(*label);
            }

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // ---- owned controls, exposed for the hosting main + tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::label& bold_label()
        {
            return bold_;
        }
        [[nodiscard]] maui::controls::label& italic_label()
        {
            return italic_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label title_;
        maui::controls::label subtitle_;
        maui::controls::label header_;
        maui::controls::label body_;
        maui::controls::label caption_;
        maui::controls::label bold_;
        maui::controls::label italic_;
        maui::controls::label bold_italic_;
        maui::controls::label kerned_;
    };
} // namespace maui::samples
