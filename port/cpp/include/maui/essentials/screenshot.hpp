#pragma once
// maui::media::screenshot           <=  Microsoft.Maui.Media.Screenshot (static facade)
// maui::media::i_screenshot         <=  Microsoft.Maui.Media.IScreenshot
// maui::media::i_screenshot_result  <=  Microsoft.Maui.Media.IScreenshotResult
// maui::media::screenshot_format    <=  Microsoft.Maui.Media.ScreenshotFormat
//
// Captures the app's current screen. CaptureAsync yields an IScreenshotResult carrying the pixel
// width/height plus a stream of the encoded image (PNG or JPEG). The C# Task surface becomes the
// library's callback convention; the result's OpenReadAsync(format, quality) becomes
// open_read_async delivering the encoded bytes (the port's stream stand-in). The facade's
// CaptureAsync gate (throw FeatureNotSupportedException when !IsCaptureSupported) is reproduced.
//
// Backends (suffix oracle): ios REAL (Screenshot.ios.cs - UIGraphicsImageRenderer over the current
// UIWindow; AsPNG/AsJPEG for the stream). apple/macOS NOT SUPPORTED
// (Screenshot.netstandard.watchos.macos.cs - IsCaptureSupported and CaptureAsync both throw; macOS
// has no shared screenshot partial). Headless mirrors netstandard (throws) until the fake is
// configured (then it reports a canned result with settable width/height/bytes).
// SIMULATOR NOTE: the ios CaptureAsync needs a key UIWindow to render; the spawned gtest process has
// none, so the on-simulator suite asserts the supported flag + the no-window error path - the real
// render is exercised only inside an app with a window.

#include <cstddef>
#include <memory>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::media
{
    // ScreenshotFormat: the encoding for reading a screenshot's stream.
    enum class screenshot_format
    {
        png = 0,
        jpeg = 1,
    };

    class i_screenshot_result
    {
    public:
        virtual ~i_screenshot_result() = default;

        // IScreenshotResult.Width / Height (pixels).
        [[nodiscard]] virtual int width() const = 0;
        [[nodiscard]] virtual int height() const = 0;

        // OpenReadAsync(format, quality): the encoded image bytes (quality 0-100 applies to JPEG).
        // The port's stream stand-in - the callback receives the full encoded buffer.
        using read_callback = maui::core::move_only_function<void(const std::vector<std::byte>&)>;
        virtual void open_read_async(screenshot_format format, int quality, read_callback on_complete) = 0;

        void open_read_async(read_callback on_complete)
        {
            open_read_async(screenshot_format::png, 100, std::move(on_complete));
        }

    protected:
        i_screenshot_result() = default;
        i_screenshot_result(const i_screenshot_result&) = default;
        i_screenshot_result(i_screenshot_result&&) = default;
        i_screenshot_result& operator=(const i_screenshot_result&) = default;
        i_screenshot_result& operator=(i_screenshot_result&&) = default;
    };

    using screenshot_callback = maui::core::move_only_function<void(const std::shared_ptr<i_screenshot_result>&)>;

    class i_screenshot
    {
    public:
        virtual ~i_screenshot() = default;

        // IScreenshot.IsCaptureSupported.
        [[nodiscard]] virtual bool is_capture_supported() const = 0;
        // IScreenshot.CaptureAsync(): renders the current screen into a result.
        virtual void capture_async(screenshot_callback on_complete) = 0;

    protected:
        i_screenshot() = default;
        i_screenshot(const i_screenshot&) = default;
        i_screenshot(i_screenshot&&) = default;
        i_screenshot& operator=(const i_screenshot&) = default;
        i_screenshot& operator=(i_screenshot&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (ScreenshotImplementation), one per backend under
        // src/platform/<backend>/essentials_screenshot.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_screenshot> make_screenshot();
    } // namespace detail

    // The static facade over screenshot::default_() (C# Screenshot.Default).
    class screenshot final
    {
    public:
        screenshot() = delete;

        [[nodiscard]] static bool is_capture_supported()
        {
            return default_().is_capture_supported();
        }
        // Screenshot.CaptureAsync(): throws feature_not_supported when capture is unsupported
        // (the C# `if (!IsCaptureSupported) throw new FeatureNotSupportedException()` gate).
        static void capture_async(screenshot_callback on_complete);

        // Screenshot.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_screenshot& default_();
        static void set_default(std::shared_ptr<i_screenshot> implementation);
    };
} // namespace maui::media
