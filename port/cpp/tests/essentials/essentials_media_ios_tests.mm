// The W2-26 networking+media suite against the REAL iOS partials, run ON the simulator: NWPathMonitor
// connectivity (passive getters + listener lifecycle), the AVSpeechSynthesizer text-to-speech (real
// locales + the shared validation), the UIGraphicsImageRenderer screenshot (IsCaptureSupported is
// true, but CaptureAsync has no key window in the gtest process so it throws the no-window error -
// the documented simulator deviation), and the MediaPicker service seam (IsCaptureSupported queries
// camera availability without throwing; the picker presentation is not drivable, so the pick/capture
// calls raise the service-seam error). The behavioral contract is covered by the headless fakes.

#import <UIKit/UIKit.h>

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
    TEST(essentials_media_ios, connectivity_passive_surface)
    {
        using namespace maui::networking;
        EXPECT_NO_THROW((void)connectivity::network_access());
        EXPECT_NO_THROW((void)connectivity::connection_profiles());
    }

    TEST(essentials_media_ios, connectivity_changed_lifecycle)
    {
        using namespace maui::networking;
        connectivity::set_current(nullptr);
        const auto token = connectivity::add_connectivity_changed([](const connectivity_changed_event_args&) {});
        EXPECT_TRUE(connectivity::remove_connectivity_changed(token));
        connectivity::set_current(nullptr);
    }

    // --- text_to_speech (real, AVSpeechSynthesizer) ---
    TEST(essentials_media_ios, text_to_speech_lists_real_locales)
    {
        using namespace maui::media;
        text_to_speech::set_default(nullptr);
        std::vector<locale> locales;
        text_to_speech::get_locales_async([&](const std::vector<locale>& got) { locales = got; });
        EXPECT_FALSE(locales.empty()); // the simulator ships AVSpeechSynthesisVoice voices
        text_to_speech::set_default(nullptr);
    }

    TEST(essentials_media_ios, text_to_speech_validates_arguments)
    {
        using namespace maui::media;
        text_to_speech::set_default(nullptr);
        EXPECT_THROW(text_to_speech::speak_async(""), std::invalid_argument);
        speech_options bad;
        bad.pitch = 3.0F; // > 2
        EXPECT_THROW(text_to_speech::speak_async("hi", bad), std::out_of_range);
        text_to_speech::set_default(nullptr);
    }

    // --- screenshot (real; CaptureAsync needs a key window the gtest process lacks) ---
    TEST(essentials_media_ios, screenshot_supported_but_no_window)
    {
        using namespace maui::media;
        screenshot::set_default(nullptr);
        EXPECT_TRUE(screenshot::is_capture_supported());
        // No key UIWindow in the spawned process -> the "Unable to find current window." error.
        EXPECT_THROW(screenshot::capture_async([](const std::shared_ptr<i_screenshot_result>&) {}), std::runtime_error);
        screenshot::set_default(nullptr);
    }

    // --- media_picker (service seam) ---
    TEST(essentials_media_ios, media_picker_capture_supported_flag)
    {
        using namespace maui::media;
        media_picker::set_default(nullptr);
        // Querying camera availability must not throw (the simulator typically reports false).
        EXPECT_NO_THROW((void)media_picker::is_capture_supported());
        media_picker::set_default(nullptr);
    }

    TEST(essentials_media_ios, media_picker_pick_is_service_seam)
    {
        using namespace maui::media;
        media_picker::set_default(nullptr);
        // No presenting view controller in the gtest process -> the service-seam error.
        EXPECT_THROW(media_picker::pick_photo_async([](const std::optional<file_result>&) {}), feature_not_supported);
        media_picker::set_default(nullptr);
    }

    // --- file_picker (service seam; UIDocumentPickerViewController needs a presenting VC) ---
    TEST(essentials_media_ios, file_picker_pick_is_service_seam)
    {
        using namespace maui::storage;
        file_picker::set_default(nullptr);
        // No key window/root VC in the gtest process -> the service-seam error before presenting.
        EXPECT_THROW(file_picker::pick_async([](const std::optional<file_result>&) {}), feature_not_supported);
        EXPECT_THROW(file_picker::pick_multiple_async([](const std::vector<file_result>&) {}), feature_not_supported);
        file_picker::set_default(nullptr);
    }

    // The iOS predefined FilePickerFileType statics carry real UTType identifiers (FilePicker.ios.cs).
    TEST(essentials_media_ios, file_type_images_resolves_on_ios)
    {
        using namespace maui::storage;
        const std::optional<std::vector<std::string>> ios_types =
            file_picker_file_type::images.try_get(maui::devices::device_platform::ios());
        ASSERT_TRUE(ios_types.has_value());
        EXPECT_FALSE(ios_types->empty());
    }

    // U13: FilePicker.ios.cs:19 evaluates `options?.FileTypes?.Value` (which THROWS feature_not_supported
    // when FileTypes is set but has no iOS entry) before presenting — it must NOT silently fall back to the
    // all-content default. A FileTypes carrying only a non-iOS (android) entry throws on pick.
    TEST(essentials_media_ios, pick_with_non_ios_file_types_throws_not_supported)
    {
        using namespace maui::storage;
        file_picker::set_default(nullptr);

        pick_options options;
        options.file_types =
            file_picker_file_type({{maui::devices::device_platform::android(), {"application/pdf"}}}); // no iOS entry

        bool threw_for_file_type = false;
        try
        {
            file_picker::pick_async(options, [](const std::optional<file_result>&) {});
        }
        catch (const feature_not_supported& ex)
        {
            // The throw must originate from the FileTypes.Value mismatch (resolved before the presenter),
            // not the no-host seam guard — distinguish by message.
            threw_for_file_type = std::string_view(ex.what()).find("file type") != std::string_view::npos;
        }
        EXPECT_TRUE(threw_for_file_type)
            << "a FileTypes without an iOS entry must throw feature_not_supported, not fall back to defaults";

        file_picker::set_default(nullptr);
    }
} // namespace
