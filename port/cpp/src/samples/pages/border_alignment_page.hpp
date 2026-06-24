#pragma once
// maui::samples::border_alignment_page — ports BorderAlignment.xaml (+ BorderAlignment.xaml.cs)
//
// The C# page demonstrates Border.HorizontalOptions: four identical Borders (red 5pt stroke,
// RoundRectangle CornerRadius 5, each wrapping a blue Grid HeightRequest=40 with a white Label) laid out
// under four headline Labels — Start, Center, End, Fill — each Border aligned the matching way
// (HorizontalOptions Start / Center / End / Fill). The page is a pure layout-alignment showcase.
//
// PORT MAPPING:
//   - each section = a headline Label + a Border. Border.Stroke="Red" StrokeThickness="5" → solid_paint
//     over colors::red with set_stroke_thickness(5); the <RoundRectangle CornerRadius="5"> →
//     graphics::shapes::round_rectangle(5) on set_stroke_shape.
//   - the blue <Grid HeightRequest="40"> with a white <Label> → grid (green/blue background via
//     solid_paint, height request 40) hosting a label whose text_color is white.
//   - the four section Labels (Start/Center/End/Fill, Style="{StaticResource Headline}") → plain labels
//     carrying the same text; the StaticResource Headline style is a markup-era resource (XAML, layer 6,
//     deferred), so it is reproduced as the label text only (best-effort — see note).
//
// note: each section sets View.HorizontalOptions (Start/Center/End/Fill) via
//       set_horizontal_layout_alignment — now settable + honored at view::arrange. The headline
//       labels visibly align; the bordered cells fill the width because a Border sizes to its
//       content and the blue grid fills (faithful content-sizing). The Headline StaticResource
//       style is markup-era (deferred) — reproduced as the label text.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree (the generic mount in app_host.hpp
// attaches handlers + hosts it).

#include <array>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class border_alignment_page
    {
    public:
        border_alignment_page()
        {
            page_.set_title("Border Alignment");
            stack_.set_spacing(12);

            // The four alignment sections (the XAML's Start / Center / End / Fill blocks), built uniformly.
            for (std::size_t i = 0; i < kSectionCount; ++i)
            {
                section& sec = sections_[i];
                const char* const name = kSectionNames[i];

                sec.headline.set_text(name); // Style="{StaticResource Headline}" — text-only (see note)

                // Border: red 5pt stroke, RoundRectangle CornerRadius=5.
                sec.bordered.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::red));
                sec.bordered.set_stroke_thickness(5);
                sec.bordered.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(5.0));

                // Blue Grid (HeightRequest=40) hosting the white-text Label.
                sec.cell.set_background(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue));
                sec.cell.set_height_request(40);

                sec.caption.set_text(name);
                sec.caption.set_text_color(maui::graphics::colors::white);

                sec.cell.add(sec.caption);
                sec.bordered.set_content(sec.cell);

                // The XAML's per-section HorizontalOptions (Start / Center / End / Fill) — the alignment IS
                // the whole point of this page. Now settable on the view surface, so the four bordered
                // sections actually align differently within the stack's width (was hardcoded Fill before).
                static constexpr std::array<maui::core::layout_alignment, kSectionCount> kAligns{
                    maui::core::layout_alignment::start, maui::core::layout_alignment::center,
                    maui::core::layout_alignment::end, maui::core::layout_alignment::fill};
                sec.headline.set_horizontal_layout_alignment(kAligns[i]);
                sec.bordered.set_horizontal_layout_alignment(kAligns[i]);

                stack_.add(sec.headline);
                stack_.add(sec.bordered);
            }

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::border& section_border(std::size_t index)
        {
            return sections_.at(index).bordered;
        }

    private:
        // One alignment section: a headline label + a red-bordered blue grid carrying a white caption.
        struct section
        {
            maui::controls::label headline;
            maui::controls::border bordered;
            maui::controls::grid cell;
            maui::controls::label caption;
        };

        static constexpr std::size_t kSectionCount = 4;
        static constexpr std::array<const char*, kSectionCount> kSectionNames{"Start", "Center", "End", "Fill"};

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        std::array<section, kSectionCount> sections_;
    };
} // namespace maui::samples
