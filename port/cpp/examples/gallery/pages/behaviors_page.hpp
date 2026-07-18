#pragma once
// maui::samples::behaviors_page — ports BehaviorsPage.xaml (+ .xaml.cs) and its companion
// Controls.Sample/Behaviors/NumericValidationBehavior.cs.
//
// The MAUI page is a VerticalStackLayout holding a headline label and an Entry that carries a
// NumericValidationBehavior in Entry.Behaviors. The behavior subscribes to the entry's TextChanged and
// recolors its TextColor: Transparent while the text parses as a System.Double, Red otherwise. This port
// reproduces that exactly, code-first:
//   - numeric_validation_behavior derives from maui::controls::typed_behavior<entry> (the port of
//     Behavior<Entry>), overriding the typed on_attached_to/on_detaching_from to connect/disconnect the
//     entry's text_changed event (the port's InputView.TextChanged seam), and recoloring text_color on
//     each change — colors::transparent when std::from_chars parses the whole string as a double, else
//     colors::red (the double.TryParse branch).
//   - the page attaches the behavior the framework way: entry.behaviors().add(shared_ptr), which attaches
//     it to the owning entry at once (VisualElement.Behaviors is pre-attached to its element).
//
// Headless-safe: text_changed is the inbound channel the handler raises on a native edit; this demo also
// drives it directly through the entry's send_text_changed so the effect is observable with no backend
// (see simulate_input). The page OWNS its whole tree; the generic mount wires every owned VIEW bottom-up.

#include <charconv>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/behavior.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/detail/charconv_compat.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    // NumericValidationBehavior.cs: a Behavior<Entry> that recolors the entry red when its text is not a
    // valid System.Double. The port subscribes to the entry's text_changed in on_attached_to and drops the
    // subscription in on_detaching_from (the §8 scoped_connection teardown stands in for C#'s -= TextChanged).
    class numeric_validation_behavior final : public maui::controls::typed_behavior<maui::controls::entry>
    {
        using maui::controls::typed_behavior<maui::controls::entry>::on_attached_to;
        using maui::controls::typed_behavior<maui::controls::entry>::on_detaching_from;

    protected:
        void on_attached_to(maui::controls::entry& target) override
        {
            // entry.TextChanged += OnEntryTextChanged. Capture the entry so the handler can recolor it
            // (C#'s ((Entry)sender!).TextColor = ...). connect_scoped wraps the token so the §8 teardown
            // (the -= TextChanged in on_detaching_from) is just a reset of the member.
            connection_ = maui::core::connect_scoped(
                target.text_changed, [&target](const std::string& /*old_value*/, const std::string& new_value) {
                    recolor(target, new_value);
                });
        }
        void on_detaching_from(maui::controls::entry& /*target*/) override
        {
            connection_ = {}; // entry.TextChanged -= OnEntryTextChanged (drop the subscription)
        }

    private:
        // double.TryParse(newText, out _): valid iff the WHOLE string parses as a double. std::from_chars
        // consumes leading sign/digits/exponent and reports the unconsumed tail in ptr; we require it to
        // reach the end with no error. An empty string is not a valid double (matches TryParse("")==false).
        static bool parses_as_double(std::string_view text)
        {
            if (text.empty())
            {
                return false;
            }
            double value = 0.0;
            const char* const begin = text.data();
            const char* const end = begin + text.size();
            // from_chars_general: the floating-point std::from_chars is `= delete`d on some libc++ (the
            // Android NDK), so route through the toolchain-compat shim (charconv_compat.hpp) — identical
            // behavior where the real one exists.
            const auto [ptr, ec] = maui::detail::from_chars_general(begin, end, value);
            return ec == std::errc{} && ptr == end;
        }

        static void recolor(maui::controls::entry& target, std::string_view new_value)
        {
            target.set_text_color(parses_as_double(new_value) ? maui::graphics::colors::transparent
                                                              : maui::graphics::colors::red);
        }

        maui::core::scoped_connection connection_;
    };

    class behaviors_page
    {
    public:
        behaviors_page()
        {
            page_.set_title("Behaviors");
            // Mirror the shared-XAML twin <VerticalStackLayout Margin="12"> (no Spacing → default 0). The
            // code-first previously set spacing 12 (wrong: spread the Label/Entry) and no margin (content sat
            // 12pt left of MAUI).
            stack_.set_margin(maui::core::thickness(12));

            // Label Text="Red when the number isn't valid" Style="Headline". The port has no app-level
            // Headline resource, so size it inline to match MAUI's Headline style (FontSize 32) — the C#
            // page shows this as a large two-line heading above the Entry.
            headline_.set_text("Red when the number isn't valid");
            headline_.set_font(maui::core::font::system_font_of_size(32.0));

            // Entry Placeholder="Enter a System.Double" with the NumericValidationBehavior attached.
            value_entry_.set_placeholder("Enter a System.Double");
            behavior_ = std::make_shared<numeric_validation_behavior>();
            // Entry.Behaviors add: the entry's pre-attached Behaviors collection attaches it at once.
            value_entry_.behaviors().add(behavior_);

            stack_.add(headline_);
            stack_.add(value_entry_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Drive the behavior with no native backend: feed text through the entry's inbound text_changed
        // channel (what a native edit would do) so the validation recolor is observable headless. Returns
        // the entry's resulting text_color so a sample main / test can assert red-vs-transparent.
        maui::graphics::color simulate_input(std::string_view text)
        {
            const std::string old_text(value_entry_.text());
            value_entry_.set_text(std::string(text));
            value_entry_.send_text_changed(old_text, text);
            return value_entry_.text_color();
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& headline()
        {
            return headline_;
        }
        [[nodiscard]] maui::controls::entry& value_entry()
        {
            return value_entry_;
        }

    private:
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label headline_;
        maui::controls::entry value_entry_;
        std::shared_ptr<numeric_validation_behavior> behavior_;
    };
} // namespace maui::samples
