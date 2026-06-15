// The W2-26 networking+media suite against the REAL macOS partials (the apple backend's lazy
// defaults): NWPathMonitor connectivity (the passive getters + the listener lifecycle), the
// NSSpeechSynthesizer text-to-speech (real locales + the shared validation), the suffix-oracle
// screenshot NOT-SUPPORTED matrix (macOS has no screenshot partial - both members throw), and the
// MediaPicker service seam (IsCaptureSupported is false on macOS; the picker UI is not drivable in
// the gtest process, so only that genuinely-testable flag is asserted - the headless fakes cover the
// behavioral contract).

#import <AppKit/AppKit.h>

#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/connectivity.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_picker.hpp"
#include "maui/essentials/file_result.hpp"
#include "maui/essentials/media_picker.hpp"
#include "maui/essentials/screenshot.hpp"
#include "maui/essentials/text_to_speech.hpp"

namespace
{
    using maui::application_model::feature_not_supported;

    // --- connectivity (real, NWPathMonitor) ---
    TEST(essentials_media_apple, connectivity_passive_surface)
    {
        using namespace maui::networking;
        // The NWPathMonitor synchronous probe must materialize a NetworkAccess without throwing
        // (the value depends on the test host's network; any enum value is acceptable).
        EXPECT_NO_THROW((void)connectivity::network_access());
        EXPECT_NO_THROW((void)connectivity::connection_profiles());
    }

    TEST(essentials_media_apple, connectivity_changed_lifecycle)
    {
        using namespace maui::networking;
        connectivity::set_current(nullptr);
        // Subscribe + unsubscribe drives StartListeners/StopListeners on the real monitor; the
        // handler may or may not fire (depends on the host), but the lifecycle must not throw.
        const auto token = connectivity::add_connectivity_changed([](const connectivity_changed_event_args&) {});
        EXPECT_TRUE(connectivity::remove_connectivity_changed(token));
        connectivity::set_current(nullptr);
    }

    // --- text_to_speech (real, NSSpeechSynthesizer) ---
    TEST(essentials_media_apple, text_to_speech_lists_real_locales)
    {
        using namespace maui::media;
        text_to_speech::set_default(nullptr);
        std::vector<locale> locales;
        text_to_speech::get_locales_async([&](const std::vector<locale>& got) { locales = got; });
        EXPECT_FALSE(locales.empty()); // macOS ships system voices
        text_to_speech::set_default(nullptr);
    }

    TEST(essentials_media_apple, text_to_speech_validates_arguments)
    {
        using namespace maui::media;
        text_to_speech::set_default(nullptr);
        EXPECT_THROW(text_to_speech::speak_async(""), std::invalid_argument);
        speech_options bad;
        bad.volume = 2.0F; // > 1
        EXPECT_THROW(text_to_speech::speak_async("hi", bad), std::out_of_range);
        text_to_speech::set_default(nullptr);
    }

    // --- screenshot (NOT supported on macOS) ---
    TEST(essentials_media_apple, screenshot_not_supported)
    {
        using namespace maui::media;
        screenshot::set_default(nullptr);
        EXPECT_THROW((void)screenshot::is_capture_supported(), feature_not_supported);
        EXPECT_THROW(screenshot::capture_async([](const std::shared_ptr<i_screenshot_result>&) {}),
                     feature_not_supported);
        screenshot::set_default(nullptr);
    }

    // --- media_picker (service seam; IsCaptureSupported false on macOS) ---
    TEST(essentials_media_apple, media_picker_capture_unsupported)
    {
        using namespace maui::media;
        media_picker::set_default(nullptr);
        EXPECT_FALSE(media_picker::is_capture_supported());
        // Capture is therefore never supported -> the gate throws.
        EXPECT_THROW(media_picker::capture_photo_async([](const std::optional<file_result>&) {}),
                     feature_not_supported);
        media_picker::set_default(nullptr);
    }

    // --- file_picker (service seam; macOS has no native partial -> never drivable headlessly) ---
    TEST(essentials_media_apple, file_picker_is_service_seam)
    {
        using namespace maui::storage;
        file_picker::set_default(nullptr);
        EXPECT_THROW(file_picker::pick_async([](const std::optional<file_result>&) {}), feature_not_supported);
        EXPECT_THROW(file_picker::pick_multiple_async([](const std::vector<file_result>&) {}), feature_not_supported);
        file_picker::set_default(nullptr);
    }
} // namespace
