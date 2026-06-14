// Unit 27 comms on the headless backend: each unconfigured fake mirrors its feature's netstandard
// partial (the Essentials.UnitTests *_Tests: SetText/HasText/GetText, Sms/PhoneDialer/Contacts/
// Clipboard fail on netstandard; the Share request validation throws ArgumentException), the shared
// validators/builders are exercised verbatim (Email.GetMailToUri's RFC2368 encoding - the largest
// behavioral oracle - plus Share/PhoneDialer validation order and the WebAuthenticatorResult query
// parser), and the configured fakes record what each facade resolved to.

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "maui/core/cancellation_token.hpp"
#include "maui/essentials/clipboard.hpp"
#include "maui/essentials/contact.hpp"
#include "maui/essentials/contacts.hpp"
#include "maui/essentials/email.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/phone_dialer.hpp"
#include "maui/essentials/semantic_screen_reader.hpp"
#include "maui/essentials/share.hpp"
#include "maui/essentials/sms.hpp"
#include "maui/essentials/web_authenticator.hpp"

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace
{
    using namespace maui::application_model;
    using namespace maui::application_model::communication;
    using namespace maui::application_model::data_transfer;
    using maui::accessibility::headless_semantic_screen_reader;
    using maui::accessibility::semantic_screen_reader;

    // ---------- clipboard ----------

    class clipboard_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            clipboard::set_default(nullptr);
        }
        void TearDown() override
        {
            clipboard::set_default(nullptr);
        }

        static std::shared_ptr<headless_clipboard> install_configured()
        {
            auto fake = std::make_shared<headless_clipboard>();
            fake->configure();
            clipboard::set_default(fake);
            return fake;
        }
    };

    // Clipboard_Tests: SetText/HasText/GetText fail on netstandard.
    TEST_F(clipboard_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(clipboard::set_text_async("Text"), feature_not_supported);
        EXPECT_THROW((void)clipboard::has_text(), feature_not_supported);
        EXPECT_THROW(clipboard::get_text_async([](const std::optional<std::string>&) {}), feature_not_supported);
    }

    TEST_F(clipboard_test, set_get_has_round_trip)
    {
        install_configured();
        EXPECT_FALSE(clipboard::has_text()); // empty initially

        bool set_completed = false;
        clipboard::set_text_async("Hello", [&set_completed] { set_completed = true; });
        EXPECT_TRUE(set_completed);
        EXPECT_TRUE(clipboard::has_text());

        std::optional<std::string> got;
        clipboard::get_text_async([&got](const std::optional<std::string>& value) { got = value; });
        ASSERT_TRUE(got.has_value());
        EXPECT_EQ(got.value_or(std::string{}), "Hello");

        // The empty-string set (the C# null -> string.Empty analog) clears HasText.
        clipboard::set_text_async("");
        EXPECT_FALSE(clipboard::has_text());
        clipboard::get_text_async([](const std::optional<std::string>& value) { EXPECT_FALSE(value.has_value()); });
    }

    // ClipboardContentChanged: first-subscribe starts the observer, last-unsubscribe stops it; the
    // raise reaches the subscriber.
    TEST_F(clipboard_test, content_changed_listener_lifecycle)
    {
        auto fake = install_configured();
        EXPECT_FALSE(fake->is_listening());

        int raised = 0;
        const auto token = clipboard::add_clipboard_content_changed([&raised] { ++raised; });
        EXPECT_TRUE(fake->is_listening());

        fake->simulate_content_changed();
        EXPECT_EQ(raised, 1);

        EXPECT_TRUE(clipboard::remove_clipboard_content_changed(token));
        EXPECT_FALSE(fake->is_listening());

        fake->simulate_content_changed(); // no live subscriber
        EXPECT_EQ(raised, 1);
    }

    // ---------- share ----------

    class share_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            share::set_default(nullptr);
        }
        void TearDown() override
        {
            share::set_default(nullptr);
        }

        static std::shared_ptr<headless_share> install_configured()
        {
            auto fake = std::make_shared<headless_share>();
            fake->configure();
            share::set_default(fake);
            return fake;
        }
    };

    // Share_Tests: text/text+title fail on netstandard (the platform request throws); empty requests
    // throw ArgumentException (the validation, which runs even unconfigured).
    TEST_F(share_test, netstandard_mirror_and_validation)
    {
        // A text request with content reaches the (unconfigured) platform request -> not implemented.
        EXPECT_THROW(share::request_async(std::string("Text"), [] {}), feature_not_supported);
        EXPECT_THROW(share::request_async(std::string("Text"), std::string("Title"), [] {}), feature_not_supported);

        // The validation throws before the platform request (invalid_argument == ArgumentException).
        EXPECT_THROW(share::request_async(share_text_request{}, [] {}), std::invalid_argument);
        EXPECT_THROW(share::request_async(share_file_request{}, [] {}), std::invalid_argument);
        EXPECT_THROW(share::request_async(share_multiple_files_request{}, [] {}), std::invalid_argument);
    }

    TEST_F(share_test, text_request_records_shaped_fields)
    {
        auto fake = install_configured();
        bool completed = false;
        share::request_async(std::string("body"), std::string("My title"), [&completed] { completed = true; });
        EXPECT_TRUE(completed);
        ASSERT_TRUE(fake->last_text_request().has_value());
        const share_text_request recorded = fake->last_text_request().value_or(share_text_request{});
        EXPECT_EQ(recorded.text, "body");
        EXPECT_EQ(recorded.title, "My title");
    }

    // A uri-only text request is valid (one of Text/Uri suffices).
    TEST_F(share_test, uri_only_text_request_is_valid)
    {
        auto fake = install_configured();
        share_text_request request;
        request.uri = "https://example.com";
        share::request_async(request, [] {});
        ASSERT_TRUE(fake->last_text_request().has_value());
        EXPECT_EQ(fake->last_text_request().value_or(share_text_request{}).uri, "https://example.com");
    }

    TEST_F(share_test, file_requests_route_to_the_right_overload)
    {
        auto fake = install_configured();
        share::request_async(share_file_request{share_file{"/tmp/a.png"}}, [] {});
        ASSERT_TRUE(fake->last_file_request().has_value());
        const share_file_request file_request = fake->last_file_request().value_or(share_file_request{});
        ASSERT_TRUE(file_request.file.has_value());
        EXPECT_EQ(file_request.file.value_or(share_file{""}).full_path(), "/tmp/a.png");

        share::request_async(share_multiple_files_request{{share_file{"/tmp/a.png"}, share_file{"/tmp/b.png"}}}, [] {});
        ASSERT_TRUE(fake->last_multiple_files_request().has_value());
        EXPECT_EQ(fake->last_multiple_files_request().value_or(share_multiple_files_request{}).files.size(), 2U);
    }

    // ---------- email (GetMailToUri is the largest behavioral oracle) ----------

    // Build an email_message. subject/body are passed as optionals only to keep the call sites
    // readable (std::nullopt = "not set"); since get_mail_to_uri skips a blank subject/body exactly
    // like an unset one (IsNullOrWhiteSpace), an unset value folds to an empty string here.
    email_message make_message(const std::optional<std::string>& subject, const std::optional<std::string>& body,
                               const std::vector<std::string>& to, const std::vector<std::string>& cc = {},
                               const std::vector<std::string>& bcc = {})
    {
        email_message message;
        message.set_subject(subject.value_or(std::string{}));
        message.set_body(body.value_or(std::string{}));
        message.to() = to;
        message.cc() = cc;
        message.bcc() = bcc;
        return message;
    }

    // Email_Tests.GetMailToUri_Returns_RFC2368_Valid_Url (the full ClassData table).
    TEST(email_test, get_mail_to_uri_rfc2368)
    {
        EXPECT_EQ(communication::detail::get_mail_to_uri(email_message{}), "mailto:?");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message(std::nullopt, "Hello", {})),
                  "mailto:?body=Hello");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message("Hello", std::nullopt, {})),
                  "mailto:?subject=Hello");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message("Hello", "Yo", {})),
                  "mailto:?subject=Hello&body=Yo");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message(std::nullopt, std::nullopt, {"john@doe.net"})),
                  "mailto:?to=john%40doe.net");
        EXPECT_EQ(
            communication::detail::get_mail_to_uri(make_message(std::nullopt, std::nullopt, {}, {"john@doe.net"})),
            "mailto:?cc=john%40doe.net");
        EXPECT_EQ(
            communication::detail::get_mail_to_uri(make_message(std::nullopt, std::nullopt, {}, {}, {"john@doe.net"})),
            "mailto:?bcc=john%40doe.net");
        EXPECT_EQ(
            communication::detail::get_mail_to_uri(make_message("Claim your free rings of power",
                                                                "Click this link to get your rings...",
                                                                {"sauron@mordor.gov.middleearth"})),
            "mailto:?to=sauron%40mordor.gov.middleearth&subject=Claim%20your%20free%20rings%20of%20power&body=Click%"
            "20this%20link%20to%20get%20your%20rings...");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message(
                      "Greetings", "Greetings Hobbits!", {"bilbo@hobbiton.shire", "frodo@hobbiton.shire"})),
                  "mailto:?to=bilbo%40hobbiton.shire,frodo%40hobbiton.shire&subject=Greetings&body=Greetings%"
                  "20Hobbits%21");
        EXPECT_EQ(communication::detail::get_mail_to_uri(
                      make_message("Big waves", "Dude, there were huge waves yesterday", {}, {"surfer@maui.net"})),
                  "mailto:?cc=surfer%40maui.net&subject=Big%20waves&body=Dude%2C%20there%20were%20huge%20waves%"
                  "20yesterday");
        EXPECT_EQ(communication::detail::get_mail_to_uri(
                      make_message("Duuuude", "Sweet", {}, {"surfer@maui.net", "dude@surf.net"})),
                  "mailto:?cc=surfer%40maui.net,dude%40surf.net&subject=Duuuude&body=Sweet");
        EXPECT_EQ(communication::detail::get_mail_to_uri(make_message(
                      "Shrubberies here!", "Ekke Ekke Ekke Ekke Ptang Zoo Boing!", {}, {}, {"knights@who.say.ni"})),
                  "mailto:?bcc=knights%40who.say.ni&subject=Shrubberies%20here%21&body=Ekke%20Ekke%20Ekke%20Ekke%"
                  "20Ptang%20Zoo%20Boing%21");
        EXPECT_EQ(
            communication::detail::get_mail_to_uri(make_message("Greetings", "Greetings Hobbits!",
                                                                {"bilbo@hobbiton.shire", "frodo@hobbiton.shire"},
                                                                {"knights@who.say.ni", "arthur@who.says.nu"})),
            "mailto:?to=bilbo%40hobbiton.shire,frodo%40hobbiton.shire&cc=knights%40who.say.ni,arthur%40who.says.nu&"
            "subject=Greetings&body=Greetings%20Hobbits%21");
    }

    // GetMailToUri_Ignores_Attachments + GetMailToUri_Ignores_BodyFormat.
    TEST(email_test, get_mail_to_uri_ignores_attachments_and_body_format)
    {
        email_message with_attachment = make_message(std::nullopt, std::nullopt, {"mom@maui.net"});
        with_attachment.attachments().emplace_back("/my/lovely/path/selfie.jpeg");
        EXPECT_EQ(communication::detail::get_mail_to_uri(with_attachment).find("selfie"), std::string::npos);

        email_message html = make_message(std::nullopt, "Hi Mom!", {"mom@maui.net"});
        html.set_body_format(email_body_format::html);
        EXPECT_NE(communication::detail::get_mail_to_uri(html).find("Hi%20Mom%21"), std::string::npos);
    }

    class email_facade_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            email::set_default(nullptr);
        }
        void TearDown() override
        {
            email::set_default(nullptr);
        }
    };

    TEST_F(email_facade_test, netstandard_mirror_and_support_gate)
    {
        EXPECT_THROW(email::compose_async([] {}), feature_not_supported); // is_compose_supported throws

        auto fake = std::make_shared<headless_email>();
        fake->set_is_compose_supported(false);
        email::set_default(fake);
        // Unsupported -> the shared gate throws feature_not_supported BEFORE recording.
        EXPECT_THROW(email::compose_async([] {}), feature_not_supported);
        EXPECT_FALSE(fake->last_message().has_value());

        fake->set_is_compose_supported(true);
        bool completed = false;
        email::compose_async("Subj", "Body", {"a@b.c"}, [&completed] { completed = true; });
        EXPECT_TRUE(completed);
        ASSERT_TRUE(fake->last_message().has_value());
        const email_message recorded = fake->last_message().value_or(email_message{});
        EXPECT_EQ(recorded.subject(), "Subj");
        ASSERT_EQ(recorded.to().size(), 1U);
        EXPECT_EQ(recorded.to().front(), "a@b.c");
    }

    // ---------- sms ----------

    class sms_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            sms::set_default(nullptr);
        }
        void TearDown() override
        {
            sms::set_default(nullptr);
        }
    };

    // Sms_Tests.Sms_Fail_On_NetStandard.
    TEST_F(sms_test, netstandard_mirror_and_support_gate)
    {
        EXPECT_THROW(sms::compose_async([] {}), feature_not_supported);

        auto fake = std::make_shared<headless_sms>();
        fake->set_is_compose_supported(false);
        sms::set_default(fake);
        EXPECT_THROW(sms::compose_async([] {}), feature_not_supported);

        fake->set_is_compose_supported(true);
        sms::compose_async(sms_message{"Hi there", "5550109999"}, [] {});
        ASSERT_TRUE(fake->last_message().has_value());
        const sms_message recorded = fake->last_message().value_or(sms_message{});
        EXPECT_EQ(recorded.body(), "Hi there");
        ASSERT_EQ(recorded.recipients().size(), 1U);
        EXPECT_EQ(recorded.recipients().front(), "5550109999");
    }

    // SmsMessage's whitespace recipient filter (the C# IsNullOrWhiteSpace ctors).
    TEST(sms_message_test, recipient_whitespace_filter)
    {
        EXPECT_TRUE(sms_message("body", "   ").recipients().empty());
        EXPECT_TRUE(sms_message("body", std::string_view{""}).recipients().empty());
        EXPECT_EQ(sms_message("body", "555").recipients().size(), 1U);

        const sms_message from_list("body", std::vector<std::string>{"a", "  ", "", "b"});
        ASSERT_EQ(from_list.recipients().size(), 2U);
        EXPECT_EQ(from_list.recipients()[0], "a");
        EXPECT_EQ(from_list.recipients()[1], "b");
    }

    // ---------- phone_dialer ----------

    class phone_dialer_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            phone_dialer::set_default(nullptr);
        }
        void TearDown() override
        {
            phone_dialer::set_default(nullptr);
        }
    };

    // PhoneDialer_Tests.Dialer_Open_Fail_On_NetStandard + the ValidateOpen order.
    TEST_F(phone_dialer_test, netstandard_mirror_and_validation_order)
    {
        EXPECT_THROW(phone_dialer::open("1234567890"), feature_not_supported);

        auto fake = std::make_shared<headless_phone_dialer>();
        fake->set_is_supported(false);
        phone_dialer::set_default(fake);
        // Blank number throws invalid_argument BEFORE the support check (the C# ValidateOpen order).
        EXPECT_THROW(phone_dialer::open("   "), std::invalid_argument);
        // A valid number on an unsupported device throws feature_not_supported.
        EXPECT_THROW(phone_dialer::open("1234567890"), feature_not_supported);
        EXPECT_FALSE(fake->last_number().has_value());

        fake->set_is_supported(true);
        EXPECT_TRUE(phone_dialer::is_supported());
        phone_dialer::open("1234567890");
        ASSERT_TRUE(fake->last_number().has_value());
        EXPECT_EQ(fake->last_number().value_or(std::string{}), "1234567890");
    }

    // ---------- contacts ----------

    class contacts_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            contacts::set_default(nullptr);
        }
        void TearDown() override
        {
            contacts::set_default(nullptr);
        }
    };

    // Contacts_Tests.Contacts_GetAll fails on netstandard.
    TEST_F(contacts_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(contacts::get_all_async([](const std::vector<contact>&) {}), feature_not_supported);
        EXPECT_THROW(contacts::pick_contact_async([](const std::optional<contact>&) {}), feature_not_supported);
    }

    TEST_F(contacts_test, get_all_and_pick_over_staged_data)
    {
        auto fake = std::make_shared<headless_contacts>();
        contact alice("1", "", "Alice", "", "Smith", "", {contact_phone{"555"}}, {contact_email{"a@b.c"}});
        fake->set_all_contacts({alice});
        fake->set_picked_contact(alice);
        contacts::set_default(fake);

        std::vector<contact> all;
        contacts::get_all_async([&all](std::vector<contact> value) { all = std::move(value); });
        ASSERT_EQ(all.size(), 1U);
        EXPECT_EQ(all.front().display_name(), "Alice Smith"); // the BuildDisplayName fallback

        std::optional<contact> picked;
        contacts::pick_contact_async([&picked](const std::optional<contact>& value) { picked = value; });
        ASSERT_TRUE(picked.has_value());
        EXPECT_EQ(picked.value_or(contact{}).given_name(), "Alice");

        // A cancelled GetAll yields no contacts (the CNContactStore cancellation analog).
        const auto flag = std::make_shared<std::atomic<bool>>(true);
        const maui::core::cancellation_token cancelled(flag);
        std::vector<contact> after_cancel{alice};
        contacts::get_all_async(cancelled,
                                [&after_cancel](std::vector<contact> value) { after_cancel = std::move(value); });
        EXPECT_TRUE(after_cancel.empty());
    }

    // Contact.DisplayName fallback rules (BuildDisplayName).
    TEST(contact_test, display_name_fallback)
    {
        EXPECT_EQ(contact("", "", "Alice", "", "Smith", "", {}, {}).display_name(), "Alice Smith");
        EXPECT_EQ(contact("", "", "Alice", "", "", "", {}, {}).display_name(), "Alice");
        EXPECT_EQ(contact("", "", "", "", "Smith", "", {}, {}).display_name(), "Smith");
        EXPECT_EQ(contact("", "", "Alice", "", "Smith", "", {}, {}, "Custom").display_name(), "Custom");
    }

    // ---------- semantic_screen_reader ----------

    class semantic_screen_reader_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            semantic_screen_reader::set_default(nullptr);
        }
        void TearDown() override
        {
            semantic_screen_reader::set_default(nullptr);
        }
    };

    TEST_F(semantic_screen_reader_test, netstandard_mirror_then_records)
    {
        EXPECT_THROW(semantic_screen_reader::announce("hi"), feature_not_supported);

        auto fake = std::make_shared<headless_semantic_screen_reader>();
        fake->configure();
        semantic_screen_reader::set_default(fake);
        semantic_screen_reader::announce("Loading complete");
        ASSERT_TRUE(fake->last_announced().has_value());
        EXPECT_EQ(fake->last_announced().value_or(std::string{}), "Loading complete");
    }

    // ---------- web_authenticator (STRETCH) ----------

    using maui::authentication::headless_web_authenticator;
    using maui::authentication::web_authenticator;
    using maui::authentication::web_authenticator_options;
    using maui::authentication::web_authenticator_result;

    class web_authenticator_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            web_authenticator::set_default(nullptr);
        }
        void TearDown() override
        {
            web_authenticator::set_default(nullptr);
        }
    };

    TEST_F(web_authenticator_test, netstandard_mirror_throws_until_configured)
    {
        web_authenticator_options options;
        options.url = "https://example.com/auth";
        options.callback_url = "myapp://callback";
        EXPECT_THROW(
            web_authenticator::authenticate_async(options, [](const std::optional<web_authenticator_result>&) {}),
            feature_not_supported);
    }

    TEST_F(web_authenticator_test, completes_canned_result_and_records_options)
    {
        auto fake = std::make_shared<headless_web_authenticator>();
        fake->set_callback_uri("myapp://callback?access_token=abc123&expires_in=3600&id_token=jwt");
        web_authenticator::set_default(fake);

        web_authenticator_options options;
        options.url = "https://example.com/auth";
        options.callback_url = "myapp://callback";

        std::optional<web_authenticator_result> result;
        web_authenticator::authenticate_async(
            options, [&result](std::optional<web_authenticator_result> value) { result = std::move(value); });
        ASSERT_TRUE(result.has_value());
        const web_authenticator_result parsed = result.value_or(web_authenticator_result{});
        EXPECT_EQ(parsed.access_token(), "abc123");
        EXPECT_EQ(parsed.id_token(), "jwt");
        ASSERT_TRUE(fake->last_options().has_value());
        EXPECT_EQ(fake->last_options().value_or(web_authenticator_options{}).url, "https://example.com/auth");

        // A cancelled token completes std::nullopt (the TaskCanceledException analog).
        const auto flag = std::make_shared<std::atomic<bool>>(true);
        const maui::core::cancellation_token cancelled(flag);
        std::optional<web_authenticator_result> cancelled_result = web_authenticator_result{};
        web_authenticator::authenticate_async(options, cancelled,
                                              [&cancelled_result](std::optional<web_authenticator_result> value) {
                                                  cancelled_result = std::move(value);
                                              });
        EXPECT_FALSE(cancelled_result.has_value());
    }

    // WebAuthenticatorResult: the WebUtils.ParseQueryString query + fragment parser.
    TEST(web_authenticator_result_test, parses_query_and_fragment)
    {
        const web_authenticator_result result("myapp://cb?access_token=tok%20en&state=xyz#refresh_token=r1&extra=a+b");
        EXPECT_EQ(result.access_token(), "tok en"); // %20 unescaped
        EXPECT_EQ(result.get("state"), "xyz");
        EXPECT_EQ(result.refresh_token(), "r1"); // from the fragment
        EXPECT_EQ(result.get("extra"), "a b");   // '+' -> space
        EXPECT_EQ(result.callback_uri(), "myapp://cb?access_token=tok%20en&state=xyz#refresh_token=r1&extra=a+b");
    }

    // ExpiresIn / RefreshTokenExpiresIn: Timestamp + N seconds (and absent => nullopt).
    TEST(web_authenticator_result_test, expiry_math)
    {
        web_authenticator_result result(
            std::map<std::string, std::string>{{"expires_in", "3600"}, {"refresh_token_expires_in", "120"}});
        const auto fixed = std::chrono::system_clock::time_point{};
        result.set_timestamp(fixed);
        ASSERT_TRUE(result.expires_in().has_value());
        EXPECT_EQ(result.expires_in().value_or(web_authenticator_result::time_point{}),
                  fixed + std::chrono::seconds(3600));
        ASSERT_TRUE(result.refresh_token_expires_in().has_value());
        EXPECT_EQ(result.refresh_token_expires_in().value_or(web_authenticator_result::time_point{}),
                  fixed + std::chrono::seconds(120));

        const web_authenticator_result empty;
        EXPECT_FALSE(empty.expires_in().has_value());
        // A non-integer value parses to nullopt (int.TryParse failure).
        web_authenticator_result bad(std::map<std::string, std::string>{{"expires_in", "soon"}});
        EXPECT_FALSE(bad.expires_in().has_value());
    }
} // namespace
