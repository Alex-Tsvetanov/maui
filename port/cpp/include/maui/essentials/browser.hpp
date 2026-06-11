#pragma once
// maui::application_model::browser                 <=  Microsoft.Maui.ApplicationModel.Browser (static facade)
// maui::application_model::i_browser               <=  Microsoft.Maui.ApplicationModel.IBrowser
// maui::application_model::browser_launch_mode     <=  Microsoft.Maui.ApplicationModel.BrowserLaunchMode
// maui::application_model::browser_launch_flags    <=  Microsoft.Maui.ApplicationModel.BrowserLaunchFlags
// maui::application_model::browser_title_mode      <=  Microsoft.Maui.ApplicationModel.BrowserTitleMode
// maui::application_model::browser_launch_options  <=  Microsoft.Maui.ApplicationModel.BrowserLaunchOptions
//
// Displays a web page, either in the system's in-app browser surface or the external default
// browser. The C# Task<bool> becomes the library's callback convention (launch_callback from the
// launcher header); URIs are strings validated like the launcher's (the `new Uri` gate of the C#
// string overloads). The Uri/string x {plain, launchMode, options} overload matrix collapses to
// the options-taking core plus the launch-mode convenience overload.
//
// Backends (suffix oracle): apple/macOS REAL (Browser.macos.cs - NSWorkspace openURL regardless
// of options; macOS has no in-app browser surface, so both launch modes open externally), ios
// REAL (Browser.ios.cs - SystemPreferred presents an SFSafariViewController over the current
// view controller with the preferred bar/control tints + sheet flags, External routes through
// the launcher). SIMULATOR-TESTABILITY NOTE: presenting SFSafariViewController needs a key
// window/root view controller, which the spawned gtest process does not have - the ios partial
// reports false when no view controller exists to present from, and the on-simulator suite
// asserts exactly that; the presentation path itself is exercised only inside a real app.
// Headless mirrors netstandard (throws) until faked (the fake records uri + options).

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "maui/essentials/launcher.hpp" // launch_callback + the shared uri validation
#include "maui/graphics/color.hpp"

namespace maui::application_model
{
    // How to launch the browser (SystemPreferred = the in-app surface where one exists).
    enum class browser_launch_mode
    {
        system_preferred = 0,
        external = 1,
    };

    // Mode for the in-app browser title (Android-only in practice).
    enum class browser_title_mode
    {
        default_ = 0, // C# BrowserTitleMode.Default ("default" is a C++ keyword)
        show = 1,
        hide = 2,
    };

    // Additional launch flags (a [Flags] enum; combine with |).
    enum class browser_launch_flags : std::uint32_t
    {
        none = 0,
        launch_adjacent = 1,       // Android only
        present_as_page_sheet = 2, // iOS only
        present_as_form_sheet = 4, // iOS only
    };

    [[nodiscard]] constexpr browser_launch_flags operator|(browser_launch_flags a, browser_launch_flags b)
    {
        return static_cast<browser_launch_flags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }
    [[nodiscard]] constexpr browser_launch_flags operator&(browser_launch_flags a, browser_launch_flags b)
    {
        return static_cast<browser_launch_flags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    // Optional settings to open the browser with (BrowserLaunchOptions.shared.cs; the nullable
    // colors become std::optional).
    struct browser_launch_options
    {
        std::optional<maui::graphics::color> preferred_toolbar_color; // iOS + Android
        std::optional<maui::graphics::color> preferred_control_color; // iOS only
        browser_launch_mode launch_mode = browser_launch_mode::system_preferred;
        browser_title_mode title_mode = browser_title_mode::default_;
        browser_launch_flags flags = browser_launch_flags::none;

        // BrowserLaunchOptions.HasFlag.
        [[nodiscard]] bool has_flag(browser_launch_flags flag) const
        {
            return (flags & flag) == flag;
        }
    };

    class i_browser
    {
    public:
        virtual ~i_browser() = default;

        // OpenAsync(uri, options): launched (not necessarily closed) -> true via the callback.
        virtual void open_async(std::string_view uri, const browser_launch_options& options,
                                launch_callback on_complete) = 0;

    protected:
        i_browser() = default;
        i_browser(const i_browser&) = default;
        i_browser(i_browser&&) = default;
        i_browser& operator=(const i_browser&) = default;
        i_browser& operator=(i_browser&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (BrowserImplementation), one per backend under
        // src/platform/<backend>/essentials_browser.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_browser> make_browser();
    } // namespace detail

    // The static facade over browser::default_() (C# Browser).
    class browser final
    {
    public:
        browser() = delete;

        static void open_async(std::string_view uri, launch_callback on_complete)
        {
            open_async(uri, browser_launch_options{}, std::move(on_complete));
        }
        static void open_async(std::string_view uri, browser_launch_mode launch_mode, launch_callback on_complete)
        {
            browser_launch_options options;
            options.launch_mode = launch_mode;
            open_async(uri, options, std::move(on_complete));
        }
        static void open_async(std::string_view uri, const browser_launch_options& options, launch_callback on_complete)
        {
            detail::require_valid_uri(uri);
            default_().open_async(uri, options, std::move(on_complete));
        }

        // Browser.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_browser& default_();
        static void set_default(std::shared_ptr<i_browser> implementation);
    };
} // namespace maui::application_model
