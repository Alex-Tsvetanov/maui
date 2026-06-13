#pragma once
// maui::media::media_picker          <=  Microsoft.Maui.Media.MediaPicker (static facade)
// maui::media::i_media_picker        <=  Microsoft.Maui.Media.IMediaPicker
// maui::media::media_picker_options  <=  Microsoft.Maui.Media.MediaPickerOptions
//
// Picks or captures a photo/video. The C# Task<FileResult?> / Task<List<FileResult>> surface
// becomes the library's callback convention: the single pick/capture callbacks receive an
// optional<file_result> (empty = the user cancelled, the C# null), the multi-pick callbacks a
// vector<file_result> (empty = cancelled). IsCaptureSupported gates CapturePhoto/CaptureVideo
// (throw FeatureNotSupportedException when false), per the ios partial.
//
// SERVICE SEAM / DEVIATION: the real photo/camera UI is NOT drivable in the spawned simulator gtest
// process (no presenting view controller / no window), so media_picker is a SERVICE SEAM - the
// apple/ios partials are present (the macos partial picks via FilePicker; the ios partial presents
// the photo library / camera) but exercised only inside a real app. The headless fake is the test
// path: it returns a canned file_result configured per pick/capture kind.
//
// Backends (suffix oracle): apple/macOS REAL (MediaPicker.macos.cs - IsCaptureSupported is false,
// pick routes through FilePicker), ios REAL (MediaPicker.ios.cs - UIImagePickerController /
// PHPickerViewController; IsCaptureSupported = camera availability). Headless mirrors netstandard
// (MediaPicker.netstandard.watchos.tvos.cs throws) until the fake is configured.

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"
#include "maui/essentials/file_result.hpp"

namespace maui::media
{
    // FileResult lives in Microsoft.Maui.Storage; expose it under maui::media for the picker results
    // (the C# MediaPicker returns Microsoft.Maui.Storage.FileResult).
    using maui::storage::file_result;

    // MediaPickerOptions: pick/capture knobs. CompressionQuality clamps to [0,100]; SelectionLimit
    // defaults to 1 (0 = no limit). The image-processing knobs (resize/rotate/metadata) are carried
    // for parity though the headless fake does not process pixels.
    class media_picker_options
    {
    public:
        // MediaPickerOptions.CompressionQuality (clamped to [0, 100]).
        [[nodiscard]] int compression_quality() const
        {
            return compression_quality_;
        }
        void set_compression_quality(int value)
        {
            compression_quality_ = value < 0 ? 0 : (value > 100 ? 100 : value);
        }

        std::optional<int> maximum_width;  // MaximumWidth (null/0 = no constraint)
        std::optional<int> maximum_height; // MaximumHeight
        std::string title;                 // Title (may be ignored by the OS)
        int selection_limit = 1;           // SelectionLimit (0 = no limit)
        bool rotate_image = false;         // RotateImage (apply EXIF orientation)
        bool preserve_metadata = true;     // PreserveMetaData

    private:
        int compression_quality_ = 100;
    };

    using file_result_callback = maui::core::move_only_function<void(const std::optional<file_result>&)>;
    using file_results_callback = maui::core::move_only_function<void(const std::vector<file_result>&)>;

    class i_media_picker
    {
    public:
        virtual ~i_media_picker() = default;

        // IMediaPicker.IsCaptureSupported.
        [[nodiscard]] virtual bool is_capture_supported() const = 0;

        // PickPhotoAsync / PickVideoAsync (single, [Obsolete] in C# but kept for surface parity).
        virtual void pick_photo_async(const media_picker_options& options, file_result_callback on_complete) = 0;
        virtual void pick_video_async(const media_picker_options& options, file_result_callback on_complete) = 0;
        // PickPhotosAsync / PickVideosAsync (multi).
        virtual void pick_photos_async(const media_picker_options& options, file_results_callback on_complete) = 0;
        virtual void pick_videos_async(const media_picker_options& options, file_results_callback on_complete) = 0;
        // CapturePhotoAsync / CaptureVideoAsync (camera; throw feature_not_supported when capture
        // is unsupported, per the ios partial's gate).
        virtual void capture_photo_async(const media_picker_options& options, file_result_callback on_complete) = 0;
        virtual void capture_video_async(const media_picker_options& options, file_result_callback on_complete) = 0;

    protected:
        i_media_picker() = default;
        i_media_picker(const i_media_picker&) = default;
        i_media_picker(i_media_picker&&) = default;
        i_media_picker& operator=(const i_media_picker&) = default;
        i_media_picker& operator=(i_media_picker&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (MediaPickerImplementation), one per backend under
        // src/platform/<backend>/essentials_media_picker.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_media_picker> make_media_picker();
    } // namespace detail

    // The static facade over media_picker::default_() (C# MediaPicker.Default). All the pick/capture
    // overloads default the options to MediaPickerOptions{}.
    class media_picker final
    {
    public:
        media_picker() = delete;

        [[nodiscard]] static bool is_capture_supported()
        {
            return default_().is_capture_supported();
        }

        static void pick_photo_async(file_result_callback on_complete)
        {
            default_().pick_photo_async(media_picker_options{}, std::move(on_complete));
        }
        static void pick_photo_async(const media_picker_options& options, file_result_callback on_complete)
        {
            default_().pick_photo_async(options, std::move(on_complete));
        }
        static void pick_video_async(file_result_callback on_complete)
        {
            default_().pick_video_async(media_picker_options{}, std::move(on_complete));
        }
        static void pick_video_async(const media_picker_options& options, file_result_callback on_complete)
        {
            default_().pick_video_async(options, std::move(on_complete));
        }
        static void pick_photos_async(file_results_callback on_complete)
        {
            default_().pick_photos_async(media_picker_options{}, std::move(on_complete));
        }
        static void pick_photos_async(const media_picker_options& options, file_results_callback on_complete)
        {
            default_().pick_photos_async(options, std::move(on_complete));
        }
        static void pick_videos_async(file_results_callback on_complete)
        {
            default_().pick_videos_async(media_picker_options{}, std::move(on_complete));
        }
        static void pick_videos_async(const media_picker_options& options, file_results_callback on_complete)
        {
            default_().pick_videos_async(options, std::move(on_complete));
        }
        static void capture_photo_async(file_result_callback on_complete)
        {
            default_().capture_photo_async(media_picker_options{}, std::move(on_complete));
        }
        static void capture_photo_async(const media_picker_options& options, file_result_callback on_complete)
        {
            default_().capture_photo_async(options, std::move(on_complete));
        }
        static void capture_video_async(file_result_callback on_complete)
        {
            default_().capture_video_async(media_picker_options{}, std::move(on_complete));
        }
        static void capture_video_async(const media_picker_options& options, file_result_callback on_complete)
        {
            default_().capture_video_async(options, std::move(on_complete));
        }

        // MediaPicker.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_media_picker& default_();
        static void set_default(std::shared_ptr<i_media_picker> implementation);
    };
} // namespace maui::media
