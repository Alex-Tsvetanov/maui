#pragma once
// maui::application_model::launcher    <=  Microsoft.Maui.ApplicationModel.Launcher (static facade)
// maui::application_model::i_launcher  <=  Microsoft.Maui.ApplicationModel.ILauncher
//
// Opens a URI through the system (deep links / custom schemes). The C# Task<bool> trio becomes
// the library's callback convention: each query/open delivers its bool result through a
// launch_callback (inline on macOS - the C# partial wraps synchronous NSWorkspace calls in
// Task.FromResult; on the main queue on iOS where openURL:completionHandler: is genuinely
// async). URIs are plain strings here (the port has no System.Uri); the facade validates the
// scheme shape ("scheme:" with an RFC 3986 scheme) and throws std::invalid_argument for a
// malformed uri - the `new Uri(...)` UriFormatException of the C# string overloads. The
// Uri-vs-string overload pairs collapse onto the string surface, and WebUtils.EscapeUri's IDN
// punycode escaping is NOT ported (callers pass pre-encoded URIs; an unencodable URI simply
// reports false).
//
// OpenAsync(OpenFileRequest) (open a FILE in another app) is out of this unit's scope and not
// ported - it rides on the FileBase model that file_system also defers.
//
// Backends (suffix oracle): apple/macOS REAL (Launcher.macos.cs - NSWorkspace
// urlForApplicationToOpenURL / openURL), ios REAL (Launcher.ios.tvos.cs - UIApplication
// canOpenURL / openURL:options:completionHandler:). Headless mirrors netstandard (throws) until
// faked (the fake records opened URIs behind a settable can-open answer).

#include <memory>
#include <string_view>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::application_model
{
    // Receives the launch/query result (true = supported / opened).
    using launch_callback = maui::core::move_only_function<void(bool)>;

    class i_launcher
    {
    public:
        virtual ~i_launcher() = default;

        // CanOpenAsync: can the device open the URI's scheme?
        virtual void can_open_async(std::string_view uri, launch_callback on_complete) = 0;
        // OpenAsync: open the app the URI targets.
        virtual void open_async(std::string_view uri, launch_callback on_complete) = 0;
        // TryOpenAsync: open only when supported (false when the scheme cannot be opened).
        virtual void try_open_async(std::string_view uri, launch_callback on_complete) = 0;

    protected:
        i_launcher() = default;
        i_launcher(const i_launcher&) = default;
        i_launcher(i_launcher&&) = default;
        i_launcher& operator=(const i_launcher&) = default;
        i_launcher& operator=(i_launcher&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (LauncherImplementation), one per backend under
        // src/platform/<backend>/essentials_launcher.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_launcher> make_launcher();

        // The C# `new Uri(uri)` format gate: requires "<scheme>:<rest>" with an RFC 3986 scheme
        // (ALPHA *(ALPHA / DIGIT / "+" / "-" / ".")); throws std::invalid_argument (the
        // UriFormatException analog) otherwise.
        void require_valid_uri(std::string_view uri);
    } // namespace detail

    // The static facade over launcher::default_() (C# Launcher). The string overloads carry the
    // C# Uri-construction validation.
    class launcher final
    {
    public:
        launcher() = delete;

        static void can_open_async(std::string_view uri, launch_callback on_complete)
        {
            detail::require_valid_uri(uri);
            default_().can_open_async(uri, std::move(on_complete));
        }
        static void open_async(std::string_view uri, launch_callback on_complete)
        {
            detail::require_valid_uri(uri);
            default_().open_async(uri, std::move(on_complete));
        }
        static void try_open_async(std::string_view uri, launch_callback on_complete)
        {
            detail::require_valid_uri(uri);
            default_().try_open_async(uri, std::move(on_complete));
        }

        // Launcher.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_launcher& default_();
        static void set_default(std::shared_ptr<i_launcher> implementation);
    };
} // namespace maui::application_model
