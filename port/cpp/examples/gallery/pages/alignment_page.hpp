#pragma once
// maui::samples::alignment_page — a faithful reproduction of the maui-compare "alignment" demo
// (ComparePages.Alignment()), the shipped-.NET-MAUI reference for the visual-parity comparison.
//
// A VerticalStackLayout (Spacing 12, Padding 16) of four sections, each a bold FontSize-20 header naming a
// LayoutOptions value followed by a Border that carries that HorizontalOptions: the Border has a red 5pt
// stroke, a RoundRectangle StrokeShape (CornerRadius 5), a FIXED WidthRequest 160 / HeightRequest 40, and
// wraps a Label (white text on a blue background, centered both ways). Because the Border has an explicit
// width, its HorizontalOptions (Start/Center/End/Fill) decides where it lands in the stack's width —
// Start pinned left, Center centered, End pinned right, Fill stretched (a Fill border with an explicit
// width renders centered, the MAUI rule the port's view<>::align_horizontal mirrors). Kept 1:1 with the
// C# reference (same controls, colors, sizes, captions, order).
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <array>
#include <cstddef>
#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class alignment_page
    {
        static constexpr std::size_t k_count = 4;
        static constexpr std::array<const char*, k_count> k_names{"Start", "Center", "End", "Fill"};
        static constexpr std::array<maui::core::layout_alignment, k_count> k_aligns{
            maui::core::layout_alignment::start, maui::core::layout_alignment::center,
            maui::core::layout_alignment::end, maui::core::layout_alignment::fill};

    public:
        alignment_page()
        {
            page_.set_title("Border Alignment"); // ComparePages: Page("Border Alignment", ...).
            stack_.set_spacing(12);
            stack_.set_padding(maui::core::thickness(16));

            const auto header_font = maui::core::font::system_font_of_size(20.0, maui::core::font_weight::bold);

            for (std::size_t i = 0; i < k_count; ++i)
            {
                section& sec = sections_.at(i);
                const char* const name = k_names.at(i);

                // The bold FontSize-20 section header.
                sec.header.set_text(name);
                sec.header.set_font(header_font);

                // The Label content: white text on a blue background, centered both ways.
                sec.caption.set_text(name);
                sec.caption.set_text_color(maui::graphics::colors::white);
                sec.caption.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
                sec.caption.set_horizontal_text_alignment(maui::core::text_alignment::center);
                sec.caption.set_vertical_text_alignment(maui::core::text_alignment::center);

                // The Border: red 5pt stroke, RoundRectangle CornerRadius 5, FIXED 160x40, carrying that
                // section's HorizontalOptions (the alignment under test) over the Label content.
                sec.bordered.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
                sec.bordered.set_stroke_thickness(5);
                sec.bordered.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(5.0));
                sec.bordered.set_width_request(160);
                sec.bordered.set_height_request(40);
                sec.bordered.set_horizontal_layout_alignment(k_aligns.at(i));
                sec.bordered.set_content(sec.caption);

                stack_.add(sec.header);
                stack_.add(sec.bordered);
            }

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::border& section_border(std::size_t index)
        {
            return sections_.at(index).bordered;
        }

    private:
        // One alignment section: a bold header + a red-bordered fixed-size cell carrying a centered caption.
        struct section
        {
            maui::controls::label header;
            maui::controls::border bordered;
            maui::controls::label caption;
        };

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        std::array<section, k_count> sections_;
    };
} // namespace maui::samples
