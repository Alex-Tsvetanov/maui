#pragma once
// maui::application_model::communication::email           <=  Microsoft.Maui.ApplicationModel.Communication.Email
// (static facade) maui::application_model::communication::i_email         <=
// Microsoft.Maui.ApplicationModel.Communication.IEmail maui::application_model::communication::email_message   <=
// Microsoft.Maui.ApplicationModel.Communication.EmailMessage maui::application_model::communication::email_attachment<=
// Microsoft.Maui.ApplicationModel.Communication.EmailAttachment
// maui::application_model::communication::email_body_format <=
// Microsoft.Maui.ApplicationModel.Communication.EmailBodyFormat
//
// Composes an email through the default mail client. The C# `Task ComposeAsync(EmailMessage?)` becomes
// the library's callback convention (a completion signal; every backend completes inline). The shared
// half carries the IsComposeSupported gate (ComposeAsync throws feature_not_supported when composing
// is unsupported - exactly the C# `if (!IsComposeSupported) throw new FeatureNotSupportedException()`)
// and the RFC2368 `mailto:` builder GetMailToUri (the most behaviorally-testable piece; ported 1:1
// incl. Uri.EscapeDataString for subject/body, comma-joined escaped recipients, and the
// to/cc/bcc/subject/body parameter order). Attachments + BodyFormat are intentionally ignored by
// GetMailToUri (the C# Parameters() never emits them - asserted by the oracle's Ignores_Attachments /
// Ignores_BodyFormat tests).
//
// Backends (suffix oracle): apple/macOS REAL (Email.macos.cs - NSWorkspace opens GetMailToUri;
// IsComposeSupported = NSWorkspace can open "mailto:"), ios REAL (Email.ios.cs -
// MFMailComposeViewController when CanSendMail else a mailto: launch; the compose UI is not
// sim-drivable, the mailto fallback is). Headless mirrors netstandard (throws until faked: the fake
// records the composed message behind a settable is-supported flag).

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::application_model::communication
{
    // A bare completion signal for compose_async (the Task with no result).
    using email_completion_callback = maui::core::move_only_function<void()>;

    // EmailBodyFormat: plain text or HTML (HTML not supported on Windows).
    enum class email_body_format
    {
        plain_text = 0,
        html = 1,
    };

    // EmailAttachment: a file attachment (FileBase-derived in C#; the port keeps the full path +
    // content type, the only fields GetMailToUri-adjacent code reads).
    class email_attachment
    {
    public:
        explicit email_attachment(std::string full_path) : full_path_(std::move(full_path))
        {
        }
        email_attachment(std::string full_path, std::string content_type)
            : full_path_(std::move(full_path)), content_type_(std::move(content_type))
        {
        }

        [[nodiscard]] const std::string& full_path() const
        {
            return full_path_;
        }
        [[nodiscard]] const std::string& content_type() const
        {
            return content_type_;
        }
        void set_content_type(std::string value)
        {
            content_type_ = std::move(value);
        }

    private:
        std::string full_path_;
        std::string content_type_;
    };

    // EmailMessage: subject/body/format + to/cc/bcc recipients + attachments.
    class email_message
    {
    public:
        email_message() = default;

        // EmailMessage(subject, body, params to): To defaults to the given recipients.
        email_message(std::string subject, std::string body, std::vector<std::string> to)
            : subject_(std::move(subject)), body_(std::move(body)), to_(std::move(to))
        {
        }

        [[nodiscard]] const std::string& subject() const
        {
            return subject_;
        }
        void set_subject(std::string value)
        {
            subject_ = std::move(value);
        }

        [[nodiscard]] const std::string& body() const
        {
            return body_;
        }
        void set_body(std::string value)
        {
            body_ = std::move(value);
        }

        [[nodiscard]] email_body_format body_format() const
        {
            return body_format_;
        }
        void set_body_format(email_body_format value)
        {
            body_format_ = value;
        }

        [[nodiscard]] std::vector<std::string>& to()
        {
            return to_;
        }
        [[nodiscard]] const std::vector<std::string>& to() const
        {
            return to_;
        }

        [[nodiscard]] std::vector<std::string>& cc()
        {
            return cc_;
        }
        [[nodiscard]] const std::vector<std::string>& cc() const
        {
            return cc_;
        }

        [[nodiscard]] std::vector<std::string>& bcc()
        {
            return bcc_;
        }
        [[nodiscard]] const std::vector<std::string>& bcc() const
        {
            return bcc_;
        }

        [[nodiscard]] std::vector<email_attachment>& attachments()
        {
            return attachments_;
        }
        [[nodiscard]] const std::vector<email_attachment>& attachments() const
        {
            return attachments_;
        }

    private:
        std::string subject_;
        std::string body_;
        email_body_format body_format_ = email_body_format::plain_text;
        std::vector<std::string> to_;
        std::vector<std::string> cc_;
        std::vector<std::string> bcc_;
        std::vector<email_attachment> attachments_;
    };

    class i_email
    {
    public:
        virtual ~i_email() = default;

        // IsComposeSupported: can this device compose an email?
        [[nodiscard]] virtual bool is_compose_supported() const = 0;
        // ComposeAsync: open the mail client (throws feature_not_supported when unsupported, before
        // PlatformComposeAsync - the shared gate).
        virtual void compose_async(const email_message& message, email_completion_callback on_complete) = 0;

    protected:
        i_email() = default;
        i_email(const i_email&) = default;
        i_email(i_email&&) = default;
        i_email& operator=(const i_email&) = default;
        i_email& operator=(i_email&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (EmailImplementation), one per backend under
        // src/platform/<backend>/essentials_email.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_email> make_email();

        // GetMailToUri(EmailMessage): the RFC2368 `mailto:?...` builder (EmailImplementation.shared).
        // Internal but tested directly (the C# test calls EmailImplementation.GetMailToUri); exposed
        // so the headless email test can assert the encoding exactly like the oracle.
        [[nodiscard]] std::string get_mail_to_uri(const email_message& message);
    } // namespace detail

    // The static facade over email::default_() (C# Email).
    class email final
    {
    public:
        email() = delete;

        // ComposeAsync() - an empty message.
        static void compose_async(email_completion_callback on_complete)
        {
            default_().compose_async(email_message{}, std::move(on_complete));
        }
        // ComposeAsync(subject, body, to).
        static void compose_async(std::string subject, std::string body, std::vector<std::string> to,
                                  email_completion_callback on_complete)
        {
            default_().compose_async(email_message{std::move(subject), std::move(body), std::move(to)},
                                     std::move(on_complete));
        }
        // ComposeAsync(message).
        static void compose_async(const email_message& message, email_completion_callback on_complete)
        {
            default_().compose_async(message, std::move(on_complete));
        }

        // Email.Default (lazy platform default) + SetDefault (the C# internal test seam made public;
        // nullptr resets to the lazy platform default).
        [[nodiscard]] static i_email& default_();
        static void set_default(std::shared_ptr<i_email> implementation);
    };
} // namespace maui::application_model::communication
