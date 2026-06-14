#pragma once
// The headless backend's comms essentials implementations (Unit 27) - one controllable FAKE per
// feature, the in-memory twins of the C# *.netstandard.*.cs partials (the sibling of
// essentials_fakes.hpp / essentials_appmodel_fakes.hpp; same family contract):
//   * UNCONFIGURED, each fake mirrors its netstandard partial byte-for-byte: members that throw
//     NotImplementedInReferenceAssemblyException there throw feature_not_supported here.
//   * The set_*/configure test setters flip it into a working in-memory device: clipboard becomes a
//     real string, share/email/sms/phone_dialer record the request, contacts stage a picked contact +
//     a list, web_authenticator completes a canned result, semantic_screen_reader records the text.
//   * Everything is inline and synchronous - async callbacks complete inline on the caller's thread.
//
// The per-feature factories (detail::make_*) in src/platform/headless/essentials_<feature>.cpp return
// these fakes as the backend's lazy defaults; tests can also instantiate them directly and install
// them through the facades' set_default seams.

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/clipboard.hpp"
#include "maui/essentials/contacts.hpp"
#include "maui/essentials/email.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/phone_dialer.hpp"
#include "maui/essentials/semantic_screen_reader.hpp"
#include "maui/essentials/share.hpp"
#include "maui/essentials/sms.hpp"
#include "maui/essentials/web_authenticator.hpp"

namespace maui::application_model
{
    namespace comms_headless_detail
    {
        // The netstandard partials' throw, shared by every unconfigured comms fake member.
        [[noreturn]] inline void throw_not_implemented()
        {
            throw maui::application_model::feature_not_supported(
                "This feature is not implemented on the headless backend until the fake is configured "
                "(the netstandard-partial mirror).");
        }
    } // namespace comms_headless_detail

    // ClipboardImplementation (netstandard): every member throws - until set_text/has-text are
    // configured. Configured, it is an in-memory pasteboard string; the listener hooks are real (so the
    // facade's first-subscribe / last-unsubscribe gate runs) and simulate_changed drives the shared
    // event - mirroring the iOS UIPasteboard.ChangedNotification path (macOS has no notification, but
    // the headless fake models the iOS contract for testability).
    class headless_clipboard final : public detail::clipboard_base
    {
    public:
        void configure()
        {
            configured_ = true;
        }

        [[nodiscard]] bool has_text() const override
        {
            require_configured();
            return !text_.empty();
        }

        void set_text_async(std::string_view text, clipboard_completion_callback on_complete) override
        {
            require_configured();
            text_ = std::string(text);
            if (on_complete)
            {
                on_complete();
            }
        }

        void get_text_async(clipboard_text_callback on_complete) override
        {
            require_configured();
            on_complete(text_.empty() ? std::nullopt : std::optional<std::string>(text_));
        }

        // Drive the shared event (the platform observer's role).
        void simulate_content_changed()
        {
            on_clipboard_content_changed();
        }

        [[nodiscard]] bool is_listening() const
        {
            return listening_;
        }

    protected:
        void start_clipboard_listeners() override
        {
            require_configured();
            listening_ = true;
        }
        void stop_clipboard_listeners() override
        {
            listening_ = false;
        }

    private:
        void require_configured() const
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
        }

        bool configured_ = false;
        bool listening_ = false;
        std::string text_;
    };
} // namespace maui::application_model

namespace maui::application_model::communication
{
    namespace comms_headless_detail = maui::application_model::comms_headless_detail;

    // EmailImplementation (netstandard): IsComposeSupported + ComposeAsync throw - until configured.
    // Configured, the support flag drives the shared gate (compose throws feature_not_supported when
    // unsupported), and a supported compose records the message.
    class headless_email final : public i_email
    {
    public:
        [[nodiscard]] bool is_compose_supported() const override
        {
            if (!supported_.has_value())
            {
                comms_headless_detail::throw_not_implemented();
            }
            return *supported_;
        }

        void compose_async(const email_message& message, email_completion_callback on_complete) override
        {
            if (!is_compose_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            last_message_ = message;
            if (on_complete)
            {
                on_complete();
            }
        }

        void set_is_compose_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] const std::optional<email_message>& last_message() const
        {
            return last_message_;
        }

    private:
        std::optional<bool> supported_;
        std::optional<email_message> last_message_;
    };

    // SmsImplementation (netstandard): IsComposeSupported + ComposeAsync throw - until configured.
    class headless_sms final : public i_sms
    {
    public:
        [[nodiscard]] bool is_compose_supported() const override
        {
            if (!supported_.has_value())
            {
                comms_headless_detail::throw_not_implemented();
            }
            return *supported_;
        }

        void compose_async(const sms_message& message, sms_completion_callback on_complete) override
        {
            if (!is_compose_supported())
            {
                throw maui::application_model::feature_not_supported();
            }
            last_message_ = message;
            if (on_complete)
            {
                on_complete();
            }
        }

        void set_is_compose_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] const std::optional<sms_message>& last_message() const
        {
            return last_message_;
        }

    private:
        std::optional<bool> supported_;
        std::optional<sms_message> last_message_;
    };

    // PhoneDialerImplementation (netstandard): IsSupported + Open throw - until configured. Configured,
    // the shared ValidateOpen gate runs (blank -> invalid_argument, unsupported -> feature_not_supported)
    // and a valid open records the dialed number.
    class headless_phone_dialer final : public i_phone_dialer
    {
    public:
        [[nodiscard]] bool is_supported() const override
        {
            if (!supported_.has_value())
            {
                comms_headless_detail::throw_not_implemented();
            }
            return *supported_;
        }

        void open(std::string_view number) override
        {
            if (!supported_.has_value())
            {
                comms_headless_detail::throw_not_implemented();
            }
            detail::validate_phone_dialer_open(number, *supported_);
            last_number_ = std::string(number);
        }

        void set_is_supported(bool value)
        {
            supported_ = value;
        }
        [[nodiscard]] const std::optional<std::string>& last_number() const
        {
            return last_number_;
        }

    private:
        std::optional<bool> supported_;
        std::optional<std::string> last_number_;
    };

    // ContactsImplementation (netstandard): both queries throw - until staged. Configured, pick reports
    // the staged optional contact (std::nullopt = the user-cancelled analog) and get_all returns the
    // staged list (a cancelled token yields no contacts, the CNContactStore cancellation analog).
    class headless_contacts final : public i_contacts
    {
    public:
        void pick_contact_async(pick_contact_callback on_complete) override
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
            on_complete(picked_);
        }

        void get_all_async(maui::core::cancellation_token token, all_contacts_callback on_complete) override
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
            last_token_cancelled_ = token.is_cancelled();
            on_complete(token.is_cancelled() ? std::vector<contact>{} : all_);
        }

        void set_picked_contact(std::optional<contact> value)
        {
            configured_ = true;
            picked_ = std::move(value);
        }
        void set_all_contacts(std::vector<contact> value)
        {
            configured_ = true;
            all_ = std::move(value);
        }
        [[nodiscard]] bool last_token_cancelled() const
        {
            return last_token_cancelled_;
        }

    private:
        bool configured_ = false;
        bool last_token_cancelled_ = false;
        std::optional<contact> picked_;
        std::vector<contact> all_;
    };
} // namespace maui::application_model::communication

namespace maui::application_model::data_transfer
{
    namespace comms_headless_detail = maui::application_model::comms_headless_detail;

    // ShareImplementation (netstandard): every PlatformRequestAsync throws - until configured. The
    // shared request validation (in the facade + detail::validate_share_request) runs BEFORE the
    // platform request either way; configured, the fake records which request variant arrived (text /
    // file / multi) plus the shaped fields the picker would receive.
    class headless_share final : public i_share
    {
    public:
        void configure()
        {
            configured_ = true;
        }

        void request_async(const share_text_request& request, share_completion_callback on_complete) override
        {
            require_configured();
            last_text_ = request;
            if (on_complete)
            {
                on_complete();
            }
        }

        void request_async(const share_file_request& request, share_completion_callback on_complete) override
        {
            require_configured();
            last_file_ = request;
            if (on_complete)
            {
                on_complete();
            }
        }

        void request_async(const share_multiple_files_request& request, share_completion_callback on_complete) override
        {
            require_configured();
            last_multi_ = request;
            if (on_complete)
            {
                on_complete();
            }
        }

        [[nodiscard]] const std::optional<share_text_request>& last_text_request() const
        {
            return last_text_;
        }
        [[nodiscard]] const std::optional<share_file_request>& last_file_request() const
        {
            return last_file_;
        }
        [[nodiscard]] const std::optional<share_multiple_files_request>& last_multiple_files_request() const
        {
            return last_multi_;
        }

    private:
        void require_configured() const
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
        }

        bool configured_ = false;
        std::optional<share_text_request> last_text_;
        std::optional<share_file_request> last_file_;
        std::optional<share_multiple_files_request> last_multi_;
    };
} // namespace maui::application_model::data_transfer

namespace maui::accessibility
{
    namespace comms_headless_detail = maui::application_model::comms_headless_detail;

    // SemanticScreenReaderImplementation (netstandard): Announce throws - until configured. Configured,
    // it records the last announced text (the headless analog of UIAccessibility.PostNotification).
    class headless_semantic_screen_reader final : public i_semantic_screen_reader
    {
    public:
        void configure()
        {
            configured_ = true;
        }

        void announce(std::string_view text) override
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
            last_announced_ = std::string(text);
        }

        [[nodiscard]] const std::optional<std::string>& last_announced() const
        {
            return last_announced_;
        }

    private:
        bool configured_ = false;
        std::optional<std::string> last_announced_;
    };
} // namespace maui::accessibility

namespace maui::authentication
{
    namespace comms_headless_detail = maui::application_model::comms_headless_detail;

    // WebAuthenticatorImplementation (netstandard): AuthenticateAsync throws - until configured. The
    // fake completes a canned result (parsed from a staged callback URI, exactly as the platform would
    // from the real redirect) and records the options it was called with; a cancelled token completes
    // std::nullopt (the TaskCanceledException analog).
    class headless_web_authenticator final : public i_web_authenticator
    {
    public:
        void authenticate_async(const web_authenticator_options& options, maui::core::cancellation_token token,
                                web_authenticator_callback on_complete) override
        {
            if (!configured_)
            {
                comms_headless_detail::throw_not_implemented();
            }
            last_options_ = options;
            if (token.is_cancelled())
            {
                on_complete(std::nullopt);
                return;
            }
            on_complete(web_authenticator_result(canned_callback_uri_));
        }

        // Stage the callback URI the canned flow "redirects" to (configures the fake).
        void set_callback_uri(std::string value)
        {
            configured_ = true;
            canned_callback_uri_ = std::move(value);
        }
        [[nodiscard]] const std::optional<web_authenticator_options>& last_options() const
        {
            return last_options_;
        }

    private:
        bool configured_ = false;
        std::string canned_callback_uri_;
        std::optional<web_authenticator_options> last_options_;
    };
} // namespace maui::authentication
