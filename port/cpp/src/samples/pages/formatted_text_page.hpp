#pragma once
// formatted_text_page — a self-contained demo page for the G1 rich-text slice: a label whose
// FormattedText is built from several styled spans (bold / italic / colored / underlined / kerned),
// plus a plain label proving the Text ⇄ FormattedText exclusivity. Code-first, backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same controls directly.
//
// Demonstrated:
//   - one label, three spans: a bold red lead-in, an italic underlined middle, a kerned tail,
//   - a second label set with plain Text (the exclusivity: setting FormattedText null-clears Text),
//   - a button-free, display-only page (label is the only control), matching the W1-04 page convention.

#include <cstdio>
#include <memory>

#include "maui/controls/content_page.hpp"
#include "maui/controls/formatted_string.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/span.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_attributes.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/graphics/color.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class formatted_text_page
    {
    public:
        formatted_text_page()
        {
            page_.set_title("Formatted text");
            stack_.set_spacing(12);

            rich_.set_font(maui::core::font::of_size("Helvetica", 16));

            auto fs = std::make_shared<maui::controls::formatted_string>();

            auto lead = std::make_shared<maui::controls::span>();
            lead->set_text("Bold red ");
            lead->set_font_attributes(maui::core::font_attributes::bold);
            lead->set_text_color(maui::graphics::color(0.86F, 0.20F, 0.27F));

            auto middle = std::make_shared<maui::controls::span>();
            middle->set_text("italic underlined ");
            middle->set_font_attributes(maui::core::font_attributes::italic);
            middle->set_text_decorations(maui::core::text_decorations::underline);

            auto tail = std::make_shared<maui::controls::span>();
            tail->set_text("k e r n e d");
            tail->set_character_spacing(2.5);

            fs->add_span(lead);
            fs->add_span(middle);
            fs->add_span(tail);
            rich_.set_formatted_text(fs);

            // A plain label — its Text path is mutually exclusive with FormattedText.
            plain_.set_text("Plain text label");

            stack_.add(rich_);
            stack_.add(plain_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, rich_, "rich_");
            gallery_attach_one(app, plain_, "plain_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& rich_label()
        {
            return rich_;
        }
        [[nodiscard]] maui::controls::label& plain_label()
        {
            return plain_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label rich_;
        maui::controls::label plain_;
    };
} // namespace maui::samples
