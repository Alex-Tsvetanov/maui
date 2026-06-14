// Unit 27 comms against the REAL macOS partials (the apple backend's lazy defaults). What is safe to
// drive in the unbundled gtest process:
//   * clipboard: the REAL NSPasteboard.generalPasteboard string round-trip (set/get/has). The general
//     pasteboard PERSISTS system-wide, so the suite snapshots it in SetUp and RESTORES it in TearDown
//     (the W1-17 NSUserDefaults-restore precedent applied to the pasteboard). macOS has no change
//     notification, so subscribing to clipboard_content_changed throws feature_not_supported.
//   * email/sms/phone_dialer: is_compose_supported / is_supported are pure NSWorkspace queries (a Mac
//     has a mail client + a tel handler, so these are typically true) - asserted as no-throw bools;
//     the GetMailToUri shared builder is asserted exactly (it is platform-independent).
//   * semantic_screen_reader: macOS is NOT SUPPORTED - announce throws feature_not_supported.
//   * contacts: PickContactAsync is NOT SUPPORTED on macOS (throws); GetAllAsync is a CNContactStore
//     read that completes (empty in an unauthorized process) - asserted as a no-throw completion.
// Deliberately NOT exercised (would open real apps / windows on the dev machine, or need UI not present
// in the test process): email/sms/phone_dialer OPEN, share's picker, web_authenticator's session - the
// headless fakes cover those contracts behaviorally.

#import <AppKit/AppKit.h>

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/clipboard.hpp"
#include "maui/essentials/contacts.hpp"
#include "maui/essentials/email.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/phone_dialer.hpp"
#include "maui/essentials/semantic_screen_reader.hpp"
#include "maui/essentials/sms.hpp"

namespace
{
    using namespace maui::application_model;
    using namespace maui::application_model::communication;
    using maui::accessibility::semantic_screen_reader;

    // ---------- clipboard (real NSPasteboard, snapshot + restore) ----------

    class comms_apple_clipboard : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            clipboard::set_default(nullptr);
            NSString* const current = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
            saved_ = current != nil ? std::optional<std::string>(current.UTF8String) : std::nullopt;
        }
        void TearDown() override
        {
            // Restore the system pasteboard to its pre-test contents.
            NSPasteboard* const pasteboard = [NSPasteboard generalPasteboard];
            [pasteboard declareTypes:@[ NSPasteboardTypeString ] owner:nil];
            [pasteboard clearContents];
            if (saved_.has_value())
            {
                const std::string& text = saved_.value();
                NSString* const restored = [[NSString alloc] initWithBytes:text.data()
                                                                    length:text.size()
                                                                  encoding:NSUTF8StringEncoding];
                [pasteboard setString:restored forType:NSPasteboardTypeString];
            }
            clipboard::set_default(nullptr);
        }

    private:
        std::optional<std::string> saved_;
    };

    TEST_F(comms_apple_clipboard, set_get_has_round_trip)
    {
        clipboard::set_text_async("maui.port.unit27.apple.clip");
        EXPECT_TRUE(clipboard::has_text());
        std::optional<std::string> got;
        clipboard::get_text_async([&got](std::optional<std::string> value) { got = std::move(value); });
        ASSERT_TRUE(got.has_value());
        EXPECT_EQ(*got, "maui.port.unit27.apple.clip");
    }

    // macOS has no pasteboard-change notification (the C# StartClipboardListeners throws).
    TEST_F(comms_apple_clipboard, content_changed_subscription_unsupported)
    {
        EXPECT_THROW((void)clipboard::add_clipboard_content_changed([] {}), feature_not_supported);
    }

    // ---------- email / sms / phone_dialer support queries (no open) ----------

    TEST(comms_apple_support, queries_do_not_throw)
    {
        email::set_default(nullptr);
        sms::set_default(nullptr);
        phone_dialer::set_default(nullptr);
        EXPECT_NO_THROW((void)email::default_().is_compose_supported());
        EXPECT_NO_THROW((void)sms::default_().is_compose_supported());
        EXPECT_NO_THROW((void)phone_dialer::is_supported());
        email::set_default(nullptr);
        sms::set_default(nullptr);
        phone_dialer::set_default(nullptr);
    }

    // The RFC2368 builder is platform-independent; assert one row on the real backend too.
    TEST(comms_apple_support, get_mail_to_uri_matches)
    {
        email_message message;
        message.set_subject("Hello");
        message.set_body("Yo");
        EXPECT_EQ(communication::detail::get_mail_to_uri(message), "mailto:?subject=Hello&body=Yo");
    }

    // ---------- semantic_screen_reader (macOS NOT supported) ----------

    TEST(comms_apple_semantic, announce_unsupported_on_macos)
    {
        semantic_screen_reader::set_default(nullptr);
        EXPECT_THROW(semantic_screen_reader::announce("hi"), feature_not_supported);
        semantic_screen_reader::set_default(nullptr);
    }

    // ---------- contacts (macOS: pick unsupported, get_all completes) ----------

    TEST(comms_apple_contacts, pick_unsupported_get_all_completes)
    {
        contacts::set_default(nullptr);
        EXPECT_THROW(contacts::pick_contact_async([](const std::optional<contact>&) {}), feature_not_supported);

        bool completed = false;
        EXPECT_NO_THROW(contacts::get_all_async([&completed](const std::vector<contact>&) { completed = true; }));
        EXPECT_TRUE(completed); // an unauthorized process completes with an empty list, not a throw
        contacts::set_default(nullptr);
    }
} // namespace
