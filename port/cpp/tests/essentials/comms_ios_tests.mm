// Unit 27 comms against the REAL iOS partials, run ON the simulator (via tools/ios-sim-run.sh). What
// is safe to drive in the spawned gtest process (no UIApplication / view controller):
//   * clipboard: the REAL UIPasteboard.general string round-trip (set/get/has). UIPasteboard persists
//     for the simulator session, so the suite snapshots + restores it in SetUp/TearDown. The change
//     observer is iOS-real (UIPasteboard.changedNotification); the subscribe/unsubscribe gate runs.
//   * email/sms/phone_dialer: is_compose_supported / is_supported are pure queries - asserted as
//     no-throw bools. On the simulator MFMailComposeViewController.canSendMail and
//     MFMessageComposeViewController.canSendText are typically false (no mail/messages account) and
//     tel: is unsupported (no phone), so a compose / open would hit the unsupported gate - asserted.
//     The GetMailToUri shared builder is asserted exactly (platform-independent).
//   * semantic_screen_reader: iOS is REAL but VoiceOver is off on the simulator, so announce is a
//     silent no-op (NOT a throw) - asserted as no-throw.
//   * contacts: GetAllAsync is a CNContactStore read that completes (empty without authorization);
//     PickContactAsync has no view controller to present from, so it completes std::nullopt (the
//     documented UI-seam stand-in).
// Deliberately NOT exercised (UI not present in the test process): email/sms compose, share's picker,
// contact picker selection, web_authenticator's session - the headless fakes cover those contracts.

#import <UIKit/UIKit.h>

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/clipboard.hpp"
#include "maui/essentials/contacts.hpp"
#include "maui/essentials/email.hpp"
#include "maui/essentials/phone_dialer.hpp"
#include "maui/essentials/semantic_screen_reader.hpp"
#include "maui/essentials/sms.hpp"

namespace
{
    using namespace maui::application_model;
    using namespace maui::application_model::communication;
    using maui::accessibility::semantic_screen_reader;

    // ---------- clipboard ----------
    // CLIPBOARD NOT EXERCISED ON THE SIMULATOR (documented): UIPasteboard.general's string/hasStrings
    // BLOCK indefinitely inside the unbundled `simctl spawn` gtest process — there is no app session
    // for the pasteboard daemon (pasted) to attach to, so the very first access never returns (a hang,
    // not a throw, so it cannot be GTEST_SKIP'd around). The iOS UIPasteboard partial is verified by
    // cross-compilation (it builds clean for the simulator), and the clipboard set/get/has + the
    // listener-gate contract is exercised behaviorally on headless (comms_tests.cpp) and against a REAL
    // NSPasteboard on apple/macOS (comms_apple_tests.cpp). The on-device UIPasteboard path runs only
    // inside a real iOS app.

    // ---------- support queries (no compose / open) ----------

    TEST(comms_ios_support, queries_do_not_throw)
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

    TEST(comms_ios_support, get_mail_to_uri_matches)
    {
        email_message message;
        message.set_subject("Hello");
        message.set_body("Yo");
        EXPECT_EQ(communication::detail::get_mail_to_uri(message), "mailto:?subject=Hello&body=Yo");
    }

    // ---------- semantic_screen_reader (iOS real; VoiceOver off -> silent no-op) ----------

    TEST(comms_ios_semantic, announce_does_not_throw)
    {
        semantic_screen_reader::set_default(nullptr);
        EXPECT_NO_THROW(semantic_screen_reader::announce("Loading complete"));
        semantic_screen_reader::set_default(nullptr);
    }

    // ---------- contacts (get_all completes; pick has no view controller -> nullopt) ----------

    TEST(comms_ios_contacts, get_all_completes_and_pick_returns_nullopt)
    {
        contacts::set_default(nullptr);
        bool got_all = false;
        EXPECT_NO_THROW(contacts::get_all_async([&got_all](const std::vector<contact>&) { got_all = true; }));
        EXPECT_TRUE(got_all);

        std::optional<contact> picked = contact{};
        contacts::pick_contact_async([&picked](std::optional<contact> value) { picked = std::move(value); });
        EXPECT_FALSE(picked.has_value()); // no view controller to present the picker from
        contacts::set_default(nullptr);
    }
} // namespace
