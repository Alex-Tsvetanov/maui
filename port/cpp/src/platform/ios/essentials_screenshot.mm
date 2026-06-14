// screenshot - iOS (UIKit) platform partial. Ported from Screenshot.ios.cs: IsCaptureSupported is
// always true; CaptureAsync finds the current key UIWindow, renders its view hierarchy into a
// UIImage via UIGraphicsImageRenderer (DrawViewHierarchy), and wraps it in a ScreenshotResult whose
// Width/Height come from the image size and whose OpenReadAsync encodes PNG (UIImagePNGRepresentation)
// or JPEG (UIImageJPEGRepresentation, quality/100). The C# uses WindowStateManager.GetCurrentUIWindow;
// the port resolves the key window from the connected foreground scenes (the modern replacement for
// the deprecated UIApplication.keyWindow).
//
// SIMULATOR NOTE: the spawned gtest process has no key window, so CaptureAsync throws the
// "Unable to find current window." error (the C# InvalidOperationException) - the on-simulator suite
// asserts IsCaptureSupported == true and that no-window error; the real render runs only in an app.

#import <UIKit/UIKit.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "maui/essentials/screenshot.hpp"

namespace maui::media
{
    namespace
    {
        std::vector<std::byte> to_byte_vector(NSData* data)
        {
            std::vector<std::byte> bytes;
            if (data != nil)
            {
                const auto* const start = static_cast<const std::byte*>(data.bytes);
                bytes.assign(start, start + data.length);
            }
            return bytes;
        }

        // WindowStateManager.GetCurrentUIWindow analog: the key window of a foreground-active scene.
        UIWindow* current_key_window()
        {
            NSArray<UIScene*>* const scenes = [UIApplication.sharedApplication.connectedScenes allObjects];
            for (NSUInteger s = 0; s < scenes.count; ++s)
            {
                UIScene* const scene = scenes[s];
                if (scene.activationState != UISceneActivationStateForegroundActive ||
                    ![scene isKindOfClass:[UIWindowScene class]])
                {
                    continue;
                }
                auto* const window_scene = static_cast<UIWindowScene*>(scene);
                NSArray<UIWindow*>* const windows = window_scene.windows;
                for (NSUInteger w = 0; w < windows.count; ++w)
                {
                    if (windows[w].isKeyWindow)
                    {
                        return windows[w];
                    }
                }
            }
            return nil;
        }

        class ios_screenshot_result final : public i_screenshot_result
        {
        public:
            explicit ios_screenshot_result(UIImage* image) : image_(image)
            {
                width_ = static_cast<int>(image.size.width);
                height_ = static_cast<int>(image.size.height);
            }

            [[nodiscard]] int width() const override
            {
                return width_;
            }
            [[nodiscard]] int height() const override
            {
                return height_;
            }

            void open_read_async(screenshot_format format, int quality, read_callback on_complete) override
            {
                NSData* const data = format == screenshot_format::jpeg
                                         ? UIImageJPEGRepresentation(image_, static_cast<CGFloat>(quality) / 100.0F)
                                         : UIImagePNGRepresentation(image_);
                on_complete(to_byte_vector(data));
            }

        private:
            UIImage* image_ = nil;
            int width_ = 0;
            int height_ = 0;
        };

        class ios_screenshot final : public i_screenshot
        {
        public:
            [[nodiscard]] bool is_capture_supported() const override
            {
                return true; // Screenshot.ios.cs
            }

            void capture_async(screenshot_callback on_complete) override
            {
                UIWindow* const window = current_key_window();
                if (window == nil)
                {
                    throw std::runtime_error("Unable to find current window."); // InvalidOperationException
                }

                UIGraphicsImageRendererFormat* const fmt = [UIGraphicsImageRendererFormat defaultFormat];
                fmt.opaque = NO;
                fmt.scale = window.screen.scale;
                auto* const renderer = [[UIGraphicsImageRenderer alloc] initWithSize:window.bounds.size format:fmt];

                UIImage* const image = [renderer imageWithActions:^(UIGraphicsImageRendererContext*) {
                  [window drawViewHierarchyInRect:window.bounds afterScreenUpdates:YES];
                }];

                on_complete(std::make_shared<ios_screenshot_result>(image != nil ? image : [[UIImage alloc] init]));
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_screenshot> make_screenshot()
        {
            return std::make_shared<ios_screenshot>();
        }
    } // namespace detail
} // namespace maui::media
