#pragma once
// maui::samples::alignment_page — demonstrates per-child layout alignment (View.HorizontalOptions /
// LayoutOptions Start/Center/End/Fill), the modern MAUI replacement for the legacy alignment knobs.
//
// Mirrors the maui-compare "alignment" demo: a vertical stack of four sections, each a bold section
// header label naming an alignment, followed by a button carrying that HorizontalOptions value. Because a
// vertical_stack_layout gives every child the full stack width, each button's horizontal_layout_alignment
// decides where it lands inside that width: Start = content-width pinned left, Center = centered, End =
// pinned right, Fill = stretched edge-to-edge. The arrange-time ComputeFrame (view<>::align_horizontal,
// ported from C# LayoutExtensions.AlignHorizontal) is what honors the value — this page is its visual
// proof. The buttons are blue-filled with a red stroke and white text so the alignment is unmistakable.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.

#include <array>
#include <memory>
#include <utility>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class alignment_page
    {
        // One section per LayoutOptions value: the header caption and the alignment it demonstrates.
        struct row
        {
            const char* caption;
            maui::core::layout_alignment alignment;
        };
        static constexpr std::array<row, 4> k_rows{{
            {"Start", maui::core::layout_alignment::start},
            {"Center", maui::core::layout_alignment::center},
            {"End", maui::core::layout_alignment::end},
            {"Fill", maui::core::layout_alignment::fill},
        }};

    public:
        alignment_page()
        {
            page_.set_title("Alignment");
            root_.set_padding(maui::core::thickness(12));
            root_.set_spacing(8);

            for (std::size_t i = 0; i < k_rows.size(); ++i)
            {
                const row& r = k_rows.at(i);

                // The section header (a bold, content-height caption above its button).
                maui::controls::label& header = headers_.at(i);
                header.set_text(r.caption);
                header.set_font(maui::core::font::system_font_of_size(20.0));
                root_.add(header);

                // The button carrying this row's HorizontalOptions — blue fill, red stroke, white text.
                maui::controls::button& btn = buttons_.at(i);
                btn.set_text(r.caption);
                btn.set_text_color(maui::graphics::colors::white);
                btn.set_background_brush(
                    std::make_shared<maui::controls::solid_color_brush>(maui::graphics::colors::blue));
                btn.set_stroke_color(maui::graphics::colors::red);
                btn.set_stroke_thickness(4.0);
                btn.set_horizontal_layout_alignment(r.alignment);
                root_.add(btn);
            }

            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the section headers + buttons first, then the
        // stack, then the page), then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            for (std::size_t i = 0; i < k_rows.size(); ++i)
            {
                gallery_attach_one(app, headers_.at(i), "alignment_header");
                gallery_attach_one(app, buttons_.at(i), "alignment_button");
            }
            gallery_attach_one(app, root_, "root_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(root_); // the stack hosts the headers + buttons
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for tests / the hosting main's bottom-up attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::button& button_at(std::size_t i)
        {
            return buttons_.at(i);
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        std::array<maui::controls::label, 4> headers_;
        std::array<maui::controls::button, 4> buttons_;
    };
} // namespace maui::samples
