// media_picker - iOS (UIKit) platform partial. SERVICE SEAM: MediaPicker.ios.cs reports
// IsCaptureSupported = UIImagePickerController.IsSourceTypeAvailable(Camera) and presents a
// PHPickerViewController (pick) / UIImagePickerController (capture) over the current view controller.
// Presenting a view controller needs a key window + root VC, which the spawned simulator gtest
// process does not have, so the picker UI is not drivable here - the on-simulator smoke asserts the
// genuinely-testable IsCaptureSupported (camera availability) and the capture gate (CapturePhoto/
// CaptureVideo throw FeatureNotSupportedException when capture is unsupported); the pick/capture
// presentation runs only inside a real app.

#import <UIKit/UIKit.h>

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
                "MediaPicker requires a presenting view controller; it is a service seam not drivable "
                "headlessly.");
        }

        class ios_media_picker final : public i_media_picker
        {
        public:
            [[nodiscard]] bool is_capture_supported() const override
            {
                // MediaPicker.ios.cs.
                return [UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera] == YES;
            }

            void pick_photo_async(const media_picker_options& /*options*/,
                                  file_result_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
            void pick_video_async(const media_picker_options& /*options*/,
                                  file_result_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
            void pick_photos_async(const media_picker_options& /*options*/,
                                   file_results_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
            void pick_videos_async(const media_picker_options& /*options*/,
                                   file_results_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
            void capture_photo_async(const media_picker_options& /*options*/,
                                     file_result_callback /*on_complete*/) override
            {
                if (!is_capture_supported())
                {
                    throw maui::application_model::feature_not_supported(); // the ios CapturePhoto gate
                }
                service_seam_unavailable();
            }
            void capture_video_async(const media_picker_options& /*options*/,
                                     file_result_callback /*on_complete*/) override
            {
                if (!is_capture_supported())
                {
                    throw maui::application_model::feature_not_supported();
                }
                service_seam_unavailable();
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_media_picker> make_media_picker()
        {
            return std::make_shared<ios_media_picker>();
        }
    } // namespace detail
} // namespace maui::media
