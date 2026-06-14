#pragma once
// maui::accessibility::semantic_screen_reader    <=  Microsoft.Maui.Accessibility.SemanticScreenReader (static facade)
// maui::accessibility::i_semantic_screen_reader  <=  Microsoft.Maui.Accessibility.ISemanticScreenReader
//
// Announces text through the OS screen reader. Announce(text) is SYNCHRONOUS (void). Lives in the new
// maui::accessibility namespace, matching the C# Microsoft.Maui.Accessibility.
//
// Backends (suffix oracle): macOS NOT SUPPORTED (SemanticScreenReader.netstandard.tvos.watchos.macos.cs
// - there is no .macos.cs partial, so the macOS backend throws feature_not_supported, exactly like the
// netstandard mirror), ios REAL (SemanticScreenReader.ios.cs - UIAccessibility.PostNotification with
// the Announcement notification, gated on VoiceOver running; when VoiceOver is off the announce is a
// silent no-op, NOT a throw). Headless mirrors netstandard (throws until faked: the fake records the
// last announced text).

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace maui::accessibility
{
    class i_semantic_screen_reader
    {
    public:
        virtual ~i_semantic_screen_reader() = default;

        // Announce(text): speak the text through the screen reader.
        virtual void announce(std::string_view text) = 0;

    protected:
        i_semantic_screen_reader() = default;
        i_semantic_screen_reader(const i_semantic_screen_reader&) = default;
        i_semantic_screen_reader(i_semantic_screen_reader&&) = default;
        i_semantic_screen_reader& operator=(const i_semantic_screen_reader&) = default;
        i_semantic_screen_reader& operator=(i_semantic_screen_reader&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (SemanticScreenReaderImplementation), one per backend under
        // src/platform/<backend>/essentials_semantic_screen_reader.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_semantic_screen_reader> make_semantic_screen_reader();
    } // namespace detail

    // The static facade over semantic_screen_reader::default_() (C# SemanticScreenReader).
    class semantic_screen_reader final
    {
    public:
        semantic_screen_reader() = delete;

        // Announce(text).
        static void announce(std::string_view text)
        {
            default_().announce(text);
        }

        // SemanticScreenReader.Default (lazy platform default) + SetDefault (the C# internal test seam
        // made public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_semantic_screen_reader& default_();
        static void set_default(std::shared_ptr<i_semantic_screen_reader> implementation);
    };
} // namespace maui::accessibility
