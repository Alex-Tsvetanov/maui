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
// note: the ONLY thing distinguishing the four sections in the XAML is HorizontalOptions
//       (Start/Center/End/Fill) on both the headline Label and the Border. The M2 view surface hardcodes
//       horizontal_layout_alignment()/vertical_layout_alignment() to Fill and exposes NO HorizontalOptions
//       setter yet (deferred to the layout-options milestone), so all four bordered sections render
//       identically here — the page faithfully reproduces the four bordered controls + their labels, and
//       the alignment differentiation is best-effort pending the options surface. Each section label is
//       suffixed with its intended alignment so the demo still reads as the four-way showcase.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree and re-hosts it bottom-up via the
// shared gallery_attach helpers (each Border is a single-content host → gallery_rehost_content).

#include <array>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

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

                stack_.add(sec.headline);
                stack_.add(sec.bordered);
            }

            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (each section's caption → grid → border, the
        // headline, then outward to the stack and the page), then re-host the tree built in the ctor.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            for (std::size_t i = 0; i < kSectionCount; ++i)
            {
                section& sec = sections_[i];
                gallery_attach_one(app, sec.caption, "caption");
                gallery_attach_one(app, sec.cell, "cell");
                gallery_attach_one(app, sec.bordered, "bordered");
                gallery_attach_one(app, sec.headline, "headline");
            }
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            for (std::size_t i = 0; i < kSectionCount; ++i)
            {
                section& sec = sections_[i];
                gallery_rehost_layout(sec.cell);      // grid hosts the white caption label
                gallery_rehost_content(sec.bordered); // border hosts the grid
            }
            gallery_rehost_layout(stack_); // outer stack hosts the four headline+border pairs
            gallery_rehost_content(page_); // page hosts the stack
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
