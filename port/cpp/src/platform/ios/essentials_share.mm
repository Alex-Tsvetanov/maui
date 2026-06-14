// share - iOS (UIKit) platform partial. Ported from Share.ios.cs: a text request shapes text (NSString)
// + uri (NSURL); a file / multiple-files request shapes file NSURLs; the items are presented through a
// UIActivityViewController on the current view controller (with the iPad popover source rect from
// PresentationSourceBounds). UI-SEAM NOTE (share.hpp): presenting the activity controller needs a
// current view controller, which the spawned gtest process lacks - so with no view controller the
// request completes without presenting (the documented stand-in); the picker is exercised only inside
// a real app. The request validation already ran in the facade. Compiled as Objective-C++ with ARC for
// the ios backend.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/essentials/share.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::data_transfer
{
    namespace
    {
        using maui::platform::apple_shared::to_ns_string;
        using maui::platform::apple_shared::to_ns_url;

        bool is_blank(const std::string& value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
        }

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

        void present_items(const share_request_base& request, NSArray* items)
        {
            UIViewController* const host = current_view_controller();
            if (host == nil || items.count == 0)
            {
                return;
            }
            UIActivityViewController* const controller = [[UIActivityViewController alloc] initWithActivityItems:items
                                                                                           applicationActivities:nil];
            if (controller.popoverPresentationController != nil)
            {
                controller.popoverPresentationController.sourceView = host.view;
                const auto& bounds = request.presentation_source_bounds;
                controller.popoverPresentationController.sourceRect =
                    CGRectMake(bounds.x, bounds.y, bounds.width, bounds.height);
            }
            [host presentViewController:controller animated:YES completion:nil];
        }

        class ios_share final : public i_share
        {
        public:
            void request_async(const share_text_request& request, share_completion_callback on_complete) override
            {
                NSMutableArray* const items = [NSMutableArray array];
                if (!is_blank(request.text))
                {
                    [items addObject:to_ns_string(request.text)];
                }
                if (!is_blank(request.uri))
                {
                    NSURL* const url = to_ns_url(request.uri);
                    if (url != nil)
                    {
                        [items addObject:url];
                    }
                }
                present_items(request, items);
                if (on_complete)
                {
                    on_complete();
                }
            }

            void request_async(const share_file_request& request, share_completion_callback on_complete) override
            {
                share_multiple_files_request multi;
                multi.title = request.title;
                multi.presentation_source_bounds = request.presentation_source_bounds;
                if (request.file.has_value())
                {
                    multi.files.push_back(request.file.value_or(share_file{""}));
                }
                request_async(multi, std::move(on_complete));
            }

            void request_async(const share_multiple_files_request& request,
                               share_completion_callback on_complete) override
            {
                NSMutableArray* const items = [NSMutableArray array];
                for (const share_file& file : request.files)
                {
                    [items addObject:[NSURL fileURLWithPath:to_ns_string(file.full_path())]];
                }
                present_items(request, items);
                if (on_complete)
                {
                    on_complete();
                }
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_share> make_share()
        {
            return std::make_shared<ios_share>();
        }
    } // namespace detail
} // namespace maui::application_model::data_transfer
