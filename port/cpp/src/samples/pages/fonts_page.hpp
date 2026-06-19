#pragma once
// maui::samples::fonts_page — ports FontsPage.xaml.
//
// The MAUI page is a ScrollView over a VerticalStackLayout of labels demonstrating font handling:
//   - a label with FontAutoScalingEnabled="False",
//   - the FontAttributes set: Bold, Italic, and Bold+Italic,
//   - the named font sizes: Default, Micro, Small, Medium, Large, Body, Header, Title, Subtitle, Caption.
//
// Port mapping (code-first, headless-safe, faithful to the port's font surface — label::set_font over
// maui::core::font):
//   - FontAttributes fold into the font's weight + slant: Bold → font_weight::bold, Italic → font_slant
//     ::italic, Bold,Italic → both (the FontExtensions.WithAttributes mapping the port already uses for
//     spans). This page builds them directly with font::system_font_of_size(size, weight, slant).
//   - FontAutoScalingEnabled="False" → font.with_auto_scaling(false) (Font.DisableScaling).
//   - the named sizes resolve through the Apple FontNamedSizeService table the port's FontSizeConverter
//     uses (FontNamedSizeService.cs): Default=17 Micro=12 Small=14 Medium=17 Large=22 Body=23 Caption=18
//     Header=23 Subtitle=28 Title=34. These exact values are applied as font sizes so the headless tree
//     matches what convert_font_size("Large") etc. would yield.
//   - character spacing (kerning) is added as a small extra demo of the font/text surface (set_character
//     _spacing), beyond the XAML, to exercise the i_text_style kerning seam.
//       note: the XAML uses Style="{StaticResource Headline}" for its two section headers; the port has no
//       app-level Headline resource here, so the headers are styled inline (bold, a larger size).
//
// The page OWNS its whole tree; attach_handlers wires every owned VIEW bottom-up. ScrollView wraps the
// stack so the long label column scrolls (ScrollView is a single-content host — gallery_rehost_content).

#include <cstdio>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class fonts_page
    {
    public:
        fonts_page()
        {
            page_.set_title("Fonts");
            stack_.set_spacing(6);

            // FontAutoScalingEnabled="False": Font.DisableScaling. A small concrete size so the flag is the
            // only thing under test, not the size.
            no_scaling_.set_text("EnableScaling disabled");
            no_scaling_.set_font(maui::core::font::system_font_of_size(14).with_auto_scaling(false));

            // ---- Font attributes section ----
            attributes_header_.set_text("Font attributes");
            attributes_header_.set_font(headline_font());

            bold_.set_text("Bold");
            bold_.set_font(maui::core::font::system_font_of_size(default_size, maui::core::font_weight::bold));

            italic_.set_text("Italics");
            italic_.set_font(maui::core::font::system_font_of_size(default_size, maui::core::font_weight::regular,
                                                                   maui::core::font_slant::italic));

            bold_italic_.set_text("Bold and italics");
            bold_italic_.set_font(maui::core::font::system_font_of_size(default_size, maui::core::font_weight::bold,
                                                                        maui::core::font_slant::italic));

            // ---- Named font sizes section (Apple FontNamedSizeService values) ----
            sizes_header_.set_text("Named font sizes");
            sizes_header_.set_font(headline_font());

            set_named("Default", named_default, default_label_);
            set_named("Micro", named_micro, micro_label_);
            set_named("Small", named_small, small_label_);
            set_named("Medium", named_medium, medium_label_);
            set_named("Large", named_large, large_label_);
            set_named("Body", named_body, body_label_);
            set_named("Header", named_header, header_label_);
            set_named("Title", named_title, title_label_);
            set_named("Subtitle", named_subtitle, subtitle_label_);
            set_named("Caption", named_caption, caption_label_);

            // ---- character spacing (kerning) extra demo ----
            kerned_.set_text("Character spacing 4.0");
            kerned_.set_font(maui::core::font::system_font_of_size(named_medium));
            kerned_.set_character_spacing(4.0);

            stack_.add(no_scaling_);
            stack_.add(attributes_header_);
            stack_.add(bold_);
            stack_.add(italic_);
            stack_.add(bold_italic_);
            stack_.add(sizes_header_);
            stack_.add(default_label_);
            stack_.add(micro_label_);
            stack_.add(small_label_);
            stack_.add(medium_label_);
            stack_.add(large_label_);
            stack_.add(body_label_);
            stack_.add(header_label_);
            stack_.add(title_label_);
            stack_.add(subtitle_label_);
            stack_.add(caption_label_);
            stack_.add(kerned_);

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host both
        // single-content hosts (the inner stack into the ScrollView, the ScrollView into the page).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, no_scaling_, "no_scaling_");
            gallery_attach_one(app, attributes_header_, "attributes_header_");
            gallery_attach_one(app, bold_, "bold_");
            gallery_attach_one(app, italic_, "italic_");
            gallery_attach_one(app, bold_italic_, "bold_italic_");
            gallery_attach_one(app, sizes_header_, "sizes_header_");
            gallery_attach_one(app, default_label_, "default_label_");
            gallery_attach_one(app, micro_label_, "micro_label_");
            gallery_attach_one(app, small_label_, "small_label_");
            gallery_attach_one(app, medium_label_, "medium_label_");
            gallery_attach_one(app, large_label_, "large_label_");
            gallery_attach_one(app, body_label_, "body_label_");
            gallery_attach_one(app, header_label_, "header_label_");
            gallery_attach_one(app, title_label_, "title_label_");
            gallery_attach_one(app, subtitle_label_, "subtitle_label_");
            gallery_attach_one(app, caption_label_, "caption_label_");
            gallery_attach_one(app, kerned_, "kerned_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroll_, "scroll_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroll_);
            gallery_rehost_content(page_);
        }

        // ---- owned controls, exposed for the hosting main + tests ----
        [[nodiscard]] maui::controls::scroll_view& scroll()
        {
            return scroll_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& bold_label()
        {
            return bold_;
        }
        [[nodiscard]] maui::controls::label& italic_label()
        {
            return italic_;
        }
        [[nodiscard]] maui::controls::label& bold_italic_label()
        {
            return bold_italic_;
        }
        [[nodiscard]] maui::controls::label& large_label()
        {
            return large_label_;
        }

    private:
        // The Apple FontNamedSizeService table (src/Controls/src/Core/Compatibility/iOS/
        // FontNamedSizeService.cs) — the resolution the port's convert_font_size uses on this backend.
        static constexpr double named_default = 17.0;
        static constexpr double named_micro = 12.0;
        static constexpr double named_small = 14.0;
        static constexpr double named_medium = 17.0;
        static constexpr double named_large = 22.0;
        static constexpr double named_body = 23.0;
        static constexpr double named_caption = 18.0;
        static constexpr double named_header = 23.0;
        static constexpr double named_subtitle = 28.0;
        static constexpr double named_title = 34.0;
        static constexpr double default_size = named_default; // FontAttributes labels use the default size

        // The two section headers stand in for Style="{StaticResource Headline}" (no app resource here).
        [[nodiscard]] static maui::core::font headline_font()
        {
            return maui::core::font::system_font_of_size(20.0, maui::core::font_weight::bold);
        }

        static void set_named(const char* text, double size, maui::controls::label& target)
        {
            target.set_text(text);
            target.set_font(maui::core::font::system_font_of_size(size));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label no_scaling_;
        maui::controls::label attributes_header_;
        maui::controls::label bold_;
        maui::controls::label italic_;
        maui::controls::label bold_italic_;
        maui::controls::label sizes_header_;
        maui::controls::label default_label_;
        maui::controls::label micro_label_;
        maui::controls::label small_label_;
        maui::controls::label medium_label_;
        maui::controls::label large_label_;
        maui::controls::label body_label_;
        maui::controls::label header_label_;
        maui::controls::label title_label_;
        maui::controls::label subtitle_label_;
        maui::controls::label caption_label_;
        maui::controls::label kerned_;
    };
} // namespace maui::samples
