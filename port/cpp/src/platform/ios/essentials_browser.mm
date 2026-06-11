// browser - iOS (UIKit) platform partial. Ported from Browser.ios.cs: External routes through
// the launcher (UIApplication openURL); SystemPreferred presents an SFSafariViewController over
// the key window's root view controller, carrying the preferred bar/control tints and the
// page/form-sheet presentation flags. SIMULATOR-TESTABILITY (browser.hpp): the spawned gtest
// process has no UIApplication/key window, so SystemPreferred completes FALSE there (no view
// controller to present from) - the C# helper resolves the controller via WindowStateManager,
// which the port lacks; the key-window probe is the documented stand-in. Compiled as
// Objective-C++ with ARC for the ios backend.

#import <Foundation/Foundation.h>
#import <SafariServices/SafariServices.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string_view>
#include <utility>

#include "maui/essentials/browser.hpp"
#include "maui/essentials/launcher.hpp"
#include "maui/graphics/color.hpp"

namespace maui::application_model
{
    namespace
    {
        UIColor* to_ui_color(const maui::graphics::color& value)
        {
            return [UIColor colorWithRed:value.red green:value.green blue:value.blue alpha:value.alpha];
        }

        // The WindowStateManager.GetCurrentUIViewController stand-in: the foreground key window's
        // root view controller (nil in the spawned test process).
        UIViewController* current_view_controller()
        {
            UIApplication* const app = [UIApplication sharedApplication];
            if (app == nil)
            {
                return nil;
            }
            for (UIWindow* window in app.windows)
            {
                if (window.isKeyWindow)
                {
                    return window.rootViewController;
                }
            }
            return nil;
        }

        class ios_browser final : public i_browser
        {
        public:
            void open_async(std::string_view uri, const browser_launch_options& options,
                            launch_callback on_complete) override
            {
                if (options.launch_mode == browser_launch_mode::external)
                {
                    // return await Launcher.Default.OpenAsync(uri).
                    launcher::default_().open_async(uri, std::move(on_complete));
                    return;
                }

                NSString* const text = [[NSString alloc] initWithBytes:uri.data()
                                                                length:uri.size()
                                                              encoding:NSUTF8StringEncoding];
                NSURL* const url = [NSURL URLWithString:text];
                UIViewController* const host = current_view_controller();
                if (url == nil || host == nil)
                {
                    on_complete(false); // nothing to present from (see the header note)
                    return;
                }

                SFSafariViewController* const safari = [[SFSafariViewController alloc] initWithURL:url];
                if (options.preferred_toolbar_color.has_value())
                {
                    safari.preferredBarTintColor = to_ui_color(*options.preferred_toolbar_color);
                }
                if (options.preferred_control_color.has_value())
                {
                    safari.preferredControlTintColor = to_ui_color(*options.preferred_control_color);
                }
                if (safari.popoverPresentationController != nil)
                {
                    safari.popoverPresentationController.sourceView = host.view;
                }
                if (options.has_flag(browser_launch_flags::present_as_form_sheet))
                {
                    safari.modalPresentationStyle = UIModalPresentationFormSheet;
                }
                else if (options.has_flag(browser_launch_flags::present_as_page_sheet))
                {
                    safari.modalPresentationStyle = UIModalPresentationPageSheet;
                }

                const auto shared_callback = std::make_shared<launch_callback>(std::move(on_complete));
                [host presentViewController:safari
                                   animated:YES
                                 completion:^{
                                   (*shared_callback)(true);
                                 }];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_browser> make_browser()
        {
            return std::make_shared<ios_browser>();
        }
    } // namespace detail
} // namespace maui::application_model
