// app_info - Apple (AppKit / macOS) platform partial. Ported from
// AppInfo.ios.tvos.watchos.macos.cs (__MACOS__ branches): the bundle info dictionary supplies
// package/name/version/build (a missing key reads as "" where C# reads null - the unbundled test
// process has none of them); requested_theme is the NSAppearance best-match (aqua/dark-aqua -
// nil/no-match -> Unspecified, dark-aqua -> Dark, else Light); requested_layout_direction maps
// NSApplication.userInterfaceLayoutDirection (the macOS partial's
// IsDeviceUILayoutDirectionRightToLeft probe; a nil NSApp reads LTR); packaging_model is always
// packaged off-Windows. show_settings_ui DEVIATION (documented in app_info.hpp): C# activates
// System Settings through a ScriptingBridge SBApplication; the port opens the
// "x-apple.systempreferences:" URL through NSWorkspace - same user outcome, no ScriptingBridge.
// Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/app_info.hpp"

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

        class apple_app_info final : public i_app_info
        {
        public:
            [[nodiscard]] std::string package_name() const override
            {
                return bundle_value("CFBundleIdentifier");
            }

            // GetBundleValue("CFBundleDisplayName") ?? GetBundleValue("CFBundleName").
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
                // The NSWorkspace deviation (C# drives com.apple.systempreferences via
                // ScriptingBridge; see the header note).
                NSURL* const settings = [NSURL URLWithString:@"x-apple.systempreferences:"];
                [[NSWorkspace sharedWorkspace] openURL:settings];
            }

            [[nodiscard]] maui::core::app_theme requested_theme() const override
            {
                NSAppearance* const appearance = [NSAppearance currentDrawingAppearance];
                NSAppearanceName const best =
                    [appearance bestMatchFromAppearancesWithNames:@[ NSAppearanceNameAqua, NSAppearanceNameDarkAqua ]];
                if (best == nil)
                {
                    return maui::core::app_theme::unspecified;
                }
                if ([best isEqualToString:NSAppearanceNameDarkAqua])
                {
                    return maui::core::app_theme::dark;
                }
                return maui::core::app_theme::light;
            }

            [[nodiscard]] app_packaging_model packaging_model() const override
            {
                return app_packaging_model::packaged;
            }

            [[nodiscard]] layout_direction requested_layout_direction() const override
            {
                NSApplication* const app = [NSApplication sharedApplication];
                return app.userInterfaceLayoutDirection == NSUserInterfaceLayoutDirectionRightToLeft
                           ? layout_direction::right_to_left
                           : layout_direction::left_to_right;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_app_info> make_app_info()
        {
            return std::make_shared<apple_app_info>();
        }
    } // namespace detail
} // namespace maui::application_model
