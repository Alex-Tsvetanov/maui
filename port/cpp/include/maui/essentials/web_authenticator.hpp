#pragma once
// maui::authentication::web_authenticator         <=  Microsoft.Maui.Authentication.WebAuthenticator (static facade)
// maui::authentication::i_web_authenticator        <=  Microsoft.Maui.Authentication.IWebAuthenticator
// maui::authentication::web_authenticator_options  <=  Microsoft.Maui.Authentication.WebAuthenticatorOptions
// maui::authentication::web_authenticator_result   <=  Microsoft.Maui.Authentication.WebAuthenticatorResult
//
// (STRETCH) An OAuth-style web auth flow: navigate to a start URL, wait for a redirect to a callback
// scheme, parse the callback URI's query/fragment into a result. The C# `Task<WebAuthenticatorResult>
// AuthenticateAsync(...)` becomes the library's callback convention (delivers the result, or signals
// cancellation through a separate path). Lives in the new maui::authentication namespace.
//
// web_authenticator_result is the behaviorally-testable piece: it parses the callback URI's query and
// fragment into a key/value map (WebUtils.ParseQueryString - '&'-split segments, '='-split pairs, '+'
// -> space in values, Uri.UnescapeDataString on both name and value, ordinal keys) and exposes the
// well-known token accessors (access_token / refresh_token / id_token) + the expires_in / refresh_
// token_expires_in timestamp math (Timestamp + N seconds). Ported 1:1 from WebAuthenticatorResult.
// shared.cs + WebUtils.shared.cs.
//
// AuthenticateAsync is a UI seam: it drives ASWebAuthenticationSession (macOS) / the iOS equivalent,
// which need a presentation anchor and an interactive browser - NOT drivable in the spawned simulator
// gtest process. So the platform Authenticate is a DOCUMENTED service seam: the contract + result
// model exist, but the on-simulator suite cannot complete a real flow; the headless fake is the
// behavioral test path (it completes a canned result, parsed exactly like the platform would).
//
// Backends (suffix oracle): apple/macOS REAL (WebAuthenticator.macos.cs - ASWebAuthenticationSession
// on macOS 10.15+; needs an NSWindow anchor), ios REAL (WebAuthenticator.ios.tvos.cs -
// ASWebAuthenticationSession / SFAuthenticationSession). Headless mirrors netstandard (throws until
// faked: the fake completes a canned web_authenticator_result).

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::authentication
{
    // WebAuthenticatorResult: the parsed callback URI - a property map + the well-known accessors.
    class web_authenticator_result
    {
    public:
        // Timestamps are a system_clock time_point (DateTimeOffset analog), captured at construction.
        using time_point = std::chrono::system_clock::time_point;

        web_authenticator_result() : timestamp_(std::chrono::system_clock::now())
        {
        }

        // WebAuthenticatorResult(Uri): parse the callback URI's query + fragment into Properties.
        explicit web_authenticator_result(std::string_view callback_uri);

        // WebAuthenticatorResult(IDictionary): adopt an existing property map (Timestamp = now).
        explicit web_authenticator_result(std::map<std::string, std::string> properties)
            : timestamp_(std::chrono::system_clock::now()), properties_(std::move(properties))
        {
        }

        [[nodiscard]] const std::string& callback_uri() const
        {
            return callback_uri_;
        }

        [[nodiscard]] time_point timestamp() const
        {
            return timestamp_;
        }
        void set_timestamp(time_point value)
        {
            timestamp_ = value;
        }

        [[nodiscard]] std::map<std::string, std::string>& properties()
        {
            return properties_;
        }
        [[nodiscard]] const std::map<std::string, std::string>& properties() const
        {
            return properties_;
        }

        // Put(key, value).
        void put(const std::string& key, std::string value)
        {
            properties_[key] = std::move(value);
        }
        // Get(key): the value, or "" when absent (the C# `default` for string).
        [[nodiscard]] std::string get(const std::string& key) const
        {
            const auto entry = properties_.find(key);
            return entry != properties_.end() ? entry->second : std::string{};
        }

        [[nodiscard]] std::string access_token() const
        {
            return get("access_token");
        }
        [[nodiscard]] std::string refresh_token() const
        {
            return get("refresh_token");
        }
        [[nodiscard]] std::string id_token() const
        {
            return get("id_token");
        }

        // ExpiresIn / RefreshTokenExpiresIn: Timestamp + N seconds, where N is the (int-parsed)
        // "expires_in" / "refresh_token_expires_in" value; std::nullopt when absent or non-integer.
        [[nodiscard]] std::optional<time_point> expires_in() const
        {
            return expiry_from("expires_in");
        }
        [[nodiscard]] std::optional<time_point> refresh_token_expires_in() const
        {
            return expiry_from("refresh_token_expires_in");
        }

    private:
        [[nodiscard]] std::optional<time_point> expiry_from(const std::string& key) const;

        std::string callback_uri_;
        time_point timestamp_;
        std::map<std::string, std::string> properties_;
    };

    // WebAuthenticatorOptions: the start URL + callback URL + iOS ephemeral-session flag.
    struct web_authenticator_options
    {
        std::string url;
        std::string callback_url;
        bool prefers_ephemeral_web_browser_session = false;
    };

    // Receives the auth result. A cancelled flow (the user cancelled, or the token cancelled) delivers
    // std::nullopt (the TaskCanceledException analog, folded to "no result" per the lib's callback
    // convention).
    using web_authenticator_callback = maui::core::move_only_function<void(std::optional<web_authenticator_result>)>;

    class i_web_authenticator
    {
    public:
        virtual ~i_web_authenticator() = default;

        // AuthenticateAsync(options [, cancellationToken]).
        virtual void authenticate_async(const web_authenticator_options& options, maui::core::cancellation_token token,
                                        web_authenticator_callback on_complete) = 0;

    protected:
        i_web_authenticator() = default;
        i_web_authenticator(const i_web_authenticator&) = default;
        i_web_authenticator(i_web_authenticator&&) = default;
        i_web_authenticator& operator=(const i_web_authenticator&) = default;
        i_web_authenticator& operator=(i_web_authenticator&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (WebAuthenticatorImplementation), one per backend under
        // src/platform/<backend>/essentials_web_authenticator.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_web_authenticator> make_web_authenticator();
    } // namespace detail

    // The static facade over web_authenticator::default_() (C# WebAuthenticator).
    class web_authenticator final
    {
    public:
        web_authenticator() = delete;

        // AuthenticateAsync(url, callbackUrl) - the WebAuthenticatorExtensions convenience.
        static void authenticate_async(std::string url, std::string callback_url,
                                       web_authenticator_callback on_complete)
        {
            web_authenticator_options options;
            options.url = std::move(url);
            options.callback_url = std::move(callback_url);
            authenticate_async(options, maui::core::cancellation_token{}, std::move(on_complete));
        }
        // AuthenticateAsync(options).
        static void authenticate_async(const web_authenticator_options& options, web_authenticator_callback on_complete)
        {
            authenticate_async(options, maui::core::cancellation_token{}, std::move(on_complete));
        }
        // AuthenticateAsync(options, cancellationToken).
        static void authenticate_async(const web_authenticator_options& options, maui::core::cancellation_token token,
                                       web_authenticator_callback on_complete)
        {
            default_().authenticate_async(options, token, std::move(on_complete));
        }

        // WebAuthenticator.Default (lazy platform default) + SetDefault (the C# internal test seam
        // made public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_web_authenticator& default_();
        static void set_default(std::shared_ptr<i_web_authenticator> implementation);
    };
} // namespace maui::authentication
