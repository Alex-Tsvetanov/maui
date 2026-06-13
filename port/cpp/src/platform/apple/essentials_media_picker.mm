// media_picker - Apple (AppKit / macOS) platform partial. SERVICE SEAM: MediaPicker.macos.cs reports
// IsCaptureSupported = false (macOS has no camera-capture path) and routes PickPhoto/PickVideo through
// FilePicker (an NSOpenPanel). Presenting NSOpenPanel needs a running app/run loop, which the spawned
// gtest process does not have, so the picking UI is not drivable here - the on-host smoke asserts the
// genuinely-testable IsCaptureSupported == false, and the pick/capture calls raise a documented
// service-seam error (no presenting context). The real picker runs only inside an app.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/media_picker.hpp"

namespace maui::media
{
    namespace
    {
        [[noreturn]] void service_seam_unavailable()
        {
            throw maui::application_model::feature_not_supported(
                "MediaPicker requires a presenting app context (NSOpenPanel); it is a service seam not "
                "drivable headlessly.");
        }

        class apple_media_picker final : public i_media_picker
        {
        public:
            [[nodiscard]] bool is_capture_supported() const override
            {
                return false; // MediaPicker.macos.cs
            }

            void pick_photo_async(const media_picker_options&, file_result_callback) override
            {
                service_seam_unavailable();
            }
            void pick_video_async(const media_picker_options&, file_result_callback) override
            {
                service_seam_unavailable();
            }
            void pick_photos_async(const media_picker_options&, file_results_callback) override
            {
                service_seam_unavailable();
            }
            void pick_videos_async(const media_picker_options&, file_results_callback) override
            {
                service_seam_unavailable();
            }
            void capture_photo_async(const media_picker_options&, file_result_callback) override
            {
                // macOS: IsCaptureSupported is false, so capture is never supported.
                throw maui::application_model::feature_not_supported();
            }
            void capture_video_async(const media_picker_options&, file_result_callback) override
            {
                throw maui::application_model::feature_not_supported();
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_media_picker> make_media_picker()
        {
            return std::make_shared<apple_media_picker>();
        }
    } // namespace detail
} // namespace maui::media
