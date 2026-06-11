// app_info - iOS (UIKit) platform partial. Ported from AppInfo.ios.tvos.watchos.macos.cs
// (__IOS__ branches): the bundle info dictionary supplies package/name/version/build (a missing
// key reads as "" where C# reads null - the spawned simulator test process is unbundled);
// show_settings_ui opens UIApplicationOpenSettingsURLString through the launcher (the C# `await
// Launcher.Default.OpenAsync(...)`); requested_theme reads the MAIN SCREEN trait collection's
// userInterfaceStyle (the non-overridden traits - light/dark/else-unspecified; the deployment
// floor is past the C# iOS-13 version gate); requested_layout_direction collapses the C#
// current-window probe to UIApplication.sharedApplication.userInterfaceLayoutDirection (the port
// has no WindowStateManager; a nil sharedApplication - the spawned test process - reads LTR).
// Compiled as Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/app_info.hpp"
#include "maui/essentials/launcher.hpp"

namespace maui::application_model
{
    namespace
    {
        std::string bundle_value(std::string_view key)
        {
            NSString* const ns_key = [[NSString alloc] initWithBytes:key.data()
                                                              length:key.size()
                                                            encoding:NSUTF8StringEncoding];
            id const value = [[NSBundle mainBundle] objectForInfoDictionaryKey:ns_key];
            const char* const utf8 = [[value description] UTF8String]; // messaging nil yields nullptr
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        class ios_app_info final : public i_app_info
        {
        public:
            [[nodiscard]] std::string package_name() const override
            {
                return bundle_value("CFBundleIdentifier");
            }

            [[nodiscard]] std::string name() const override
            {
                const std::string display_name = bundle_value("CFBundleDisplayName");
                return !display_name.empty() ? display_name : bundle_value("CFBundleName");
            }

            [[nodiscard]] std::string version_string() const override
            {
                return bundle_value("CFBundleShortVersionString");
            }

            [[nodiscard]] std::string build_string() const override
            {
                return bundle_value("CFBundleVersion");
            }

            void show_settings_ui() override
            {
                // await Launcher.Default.OpenAsync(UIApplication.OpenSettingsUrlString).
                const char* const url = [UIApplicationOpenSettingsURLString UTF8String];
                launcher::default_().open_async(url != nullptr ? url : "", [](bool) {});
            }

            [[nodiscard]] maui::core::app_theme requested_theme() const override
            {
                // "This always returns the non-overridden traits" - the main screen's collection.
                const UIUserInterfaceStyle style = [UIScreen mainScreen].traitCollection.userInterfaceStyle;
                switch (style)
                {
                    case UIUserInterfaceStyleLight:
                        return maui::core::app_theme::light;
                    case UIUserInterfaceStyleDark:
                        return maui::core::app_theme::dark;
                    default:
                        return maui::core::app_theme::unspecified;
                }
            }

            [[nodiscard]] app_packaging_model packaging_model() const override
            {
                return app_packaging_model::packaged;
            }

            [[nodiscard]] layout_direction requested_layout_direction() const override
            {
                UIApplication* const app = [UIApplication sharedApplication];
                return app.userInterfaceLayoutDirection == UIUserInterfaceLayoutDirectionRightToLeft
                           ? layout_direction::right_to_left
                           : layout_direction::left_to_right;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_app_info> make_app_info()
        {
            return std::make_shared<ios_app_info>();
        }
    } // namespace detail
} // namespace maui::application_model
