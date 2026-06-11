// app_actions - iOS (UIKit) platform partial. Ported 1:1 from AppActions.ios.cs: shortcut items
// are supported (is_supported = true); get/set round-trip UIApplication.shortcutItems through
// "XE_APP_ACTION_TYPE" UIApplicationShortcutItems whose userInfo carries id/icon, the localized
// title/subtitle carry title/subtitle, and an icon becomes a template-image shortcut icon.
// perform_action_for_shortcut_item (the IPlatformAppActions activation seam) raises the shared
// event for items of the shortcut type - the UIApplicationDelegate wiring that calls it belongs
// to the app host, which the port's test process does not have. SIMULATOR-TESTABILITY
// (app_actions.hpp): with a nil [UIApplication sharedApplication] the get reads empty and the
// set no-ops - the real round-trip needs a running app. Compiled as Objective-C++ with ARC for
// the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/essentials/app_actions.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/essentials/detail/app_actions_base.hpp"

namespace maui::application_model
{
    namespace
    {
        // AppActionsImplementation.ShortcutType.
        NSString* const shortcut_type = @"XE_APP_ACTION_TYPE";

        NSString* to_ns_string(std::string_view value)
        {
            return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
        }

        std::string to_std_string(NSString* value)
        {
            const char* const utf8 = [value UTF8String]; // messaging nil yields nullptr
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        std::optional<std::string> to_optional_string(NSString* value)
        {
            return value != nil ? std::optional<std::string>(to_std_string(value)) : std::nullopt;
        }

        // AppActionsExtensions.ToAppAction(UIApplicationShortcutItem).
        app_action to_app_action(UIApplicationShortcutItem* item)
        {
            NSString* const identifier = item.userInfo[@"id"] != nil ? [item.userInfo[@"id"] description] : nil;
            NSString* const icon = item.userInfo[@"icon"] != nil ? [item.userInfo[@"icon"] description] : nil;
            return app_action(to_std_string(identifier), to_std_string(item.localizedTitle),
                              to_optional_string(item.localizedSubtitle), to_optional_string(icon));
        }

        // AppActionsExtensions.ToShortcutItem(AppAction).
        UIApplicationShortcutItem* to_shortcut_item(const app_action& action)
        {
            NSMutableDictionary<NSString*, id>* const user_info = [NSMutableDictionary dictionary];
            user_info[@"id"] = to_ns_string(action.id());
            if (action.icon().has_value() && !action.icon()->empty())
            {
                user_info[@"icon"] = to_ns_string(*action.icon());
            }
            UIApplicationShortcutIcon* const icon =
                action.icon().has_value()
                    ? [UIApplicationShortcutIcon iconWithTemplateImageName:to_ns_string(*action.icon())]
                    : nil;
            return [[UIApplicationShortcutItem alloc]
                     initWithType:shortcut_type
                   localizedTitle:to_ns_string(action.title())
                localizedSubtitle:(action.subtitle().has_value() ? to_ns_string(*action.subtitle()) : nil)icon:icon
                         userInfo:user_info];
        }

        class ios_app_actions final : public detail::app_actions_base
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                return true;
            }

            void get_async(app_actions_callback on_complete) override
            {
                std::vector<app_action> result;
                UIApplication* const app = [UIApplication sharedApplication];
                NSArray<UIApplicationShortcutItem*>* const items = app.shortcutItems; // nil app -> nil
                result.reserve(items.count);
                for (UIApplicationShortcutItem* item in items)
                {
                    result.push_back(to_app_action(item));
                }
                on_complete(result);
            }

            void set_async(const std::vector<app_action>& actions) override
            {
                NSMutableArray<UIApplicationShortcutItem*>* const items =
                    [NSMutableArray arrayWithCapacity:actions.size()];
                for (const app_action& action : actions)
                {
                    [items addObject:to_shortcut_item(action)];
                }
                UIApplication* const app = [UIApplication sharedApplication];
                app.shortcutItems = items; // nil app -> no-op (the spawned test process)
            }

            // PerformActionForShortcutItem: raise only for our shortcut type.
            void perform_action_for_shortcut_item(UIApplicationShortcutItem* item)
            {
                if ([item.type isEqualToString:shortcut_type])
                {
                    raise_app_action_activated(to_app_action(item));
                }
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_app_actions> make_app_actions()
        {
            return std::make_shared<ios_app_actions>();
        }
    } // namespace detail
} // namespace maui::application_model
