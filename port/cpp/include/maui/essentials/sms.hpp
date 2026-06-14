#pragma once
// maui::application_model::communication::sms          <=  Microsoft.Maui.ApplicationModel.Communication.Sms (static
// facade) maui::application_model::communication::i_sms        <=  Microsoft.Maui.ApplicationModel.Communication.ISms
// maui::application_model::communication::sms_message  <=  Microsoft.Maui.ApplicationModel.Communication.SmsMessage
//
// Opens the default SMS app with a prefilled message. The C# `Task ComposeAsync(SmsMessage?)` becomes
// the library's callback convention (a completion signal; backends complete inline). The shared half
// carries the IsComposeSupported gate (ComposeAsync throws feature_not_supported when unsupported -
// the C# `if (!IsComposeSupported) throw new FeatureNotSupportedException()`) and the C# null/recipient
// coalescing (a null message becomes an empty SmsMessage with an empty recipient list before the
// platform compose runs).
//
// SmsMessage's recipient ctors apply the C# whitespace filter: SmsMessage(body, recipient) skips a
// blank single recipient; SmsMessage(body, recipients) adds only the non-blank entries (the
// IsNullOrWhiteSpace filter) - ported here so the model matches.
//
// Backends (suffix oracle): apple/macOS REAL (Sms.macos.cs - NSWorkspace opens an
// "sms:/open?addresses=...&body=..." URL; IsComposeSupported = NSWorkspace can open "sms:"), ios REAL
// (Sms.ios.cs - MFMessageComposeViewController; the compose UI is NOT sim-drivable, so the on-simulator
// suite asserts the support/unsupported gate, not a sent message). Headless mirrors netstandard (throws
// until faked: the fake records the composed message behind a settable is-supported flag).

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::application_model::communication
{
    // A bare completion signal for compose_async (the Task with no result).
    using sms_completion_callback = maui::core::move_only_function<void()>;

    // SmsMessage: a body + recipients.
    class sms_message
    {
    public:
        sms_message() = default;

        // SmsMessage(body, recipient): a single recipient, skipped when blank.
        sms_message(std::string body, std::string_view recipient) : body_(std::move(body))
        {
            if (!is_blank(recipient))
            {
                recipients_.emplace_back(recipient);
            }
        }

        // SmsMessage(body, recipients): only the non-blank recipients are added.
        sms_message(std::string body, const std::vector<std::string>& recipients) : body_(std::move(body))
        {
            for (const std::string& recipient : recipients)
            {
                if (!is_blank(recipient))
                {
                    recipients_.push_back(recipient);
                }
            }
        }

        [[nodiscard]] const std::string& body() const
        {
            return body_;
        }
        void set_body(std::string value)
        {
            body_ = std::move(value);
        }

        [[nodiscard]] std::vector<std::string>& recipients()
        {
            return recipients_;
        }
        [[nodiscard]] const std::vector<std::string>& recipients() const
        {
            return recipients_;
        }

    private:
        // string.IsNullOrWhiteSpace: empty or all-whitespace.
        [[nodiscard]] static bool is_blank(std::string_view value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos;
        }

        std::string body_;
        std::vector<std::string> recipients_;
    };

    class i_sms
    {
    public:
        virtual ~i_sms() = default;

        // IsComposeSupported: can this device compose an SMS?
        [[nodiscard]] virtual bool is_compose_supported() const = 0;
        // ComposeAsync: open the SMS client (throws feature_not_supported when unsupported, before
        // PlatformComposeAsync).
        virtual void compose_async(const sms_message& message, sms_completion_callback on_complete) = 0;

    protected:
        i_sms() = default;
        i_sms(const i_sms&) = default;
        i_sms(i_sms&&) = default;
        i_sms& operator=(const i_sms&) = default;
        i_sms& operator=(i_sms&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (SmsImplementation), one per backend under
        // src/platform/<backend>/essentials_sms.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_sms> make_sms();
    } // namespace detail

    // The static facade over sms::default_() (C# Sms).
    class sms final
    {
    public:
        sms() = delete;

        // ComposeAsync() - an empty message.
        static void compose_async(sms_completion_callback on_complete)
        {
            default_().compose_async(sms_message{}, std::move(on_complete));
        }
        // ComposeAsync(message).
        static void compose_async(const sms_message& message, sms_completion_callback on_complete)
        {
            default_().compose_async(message, std::move(on_complete));
        }

        // Sms.Default (lazy platform default) + SetDefault (the C# internal test seam made public;
        // nullptr resets to the lazy platform default).
        [[nodiscard]] static i_sms& default_();
        static void set_default(std::shared_ptr<i_sms> implementation);
    };
} // namespace maui::application_model::communication
