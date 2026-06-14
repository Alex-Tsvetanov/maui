// share - Apple (AppKit / macOS) platform partial. Ported from Share.macos.cs: a text request shapes
// title + text (NSString) + uri (NSURL) into the share items; a file / multiple-files request shapes
// title + file NSURLs; the items are presented through an NSSharingServicePicker shown relative to the
// current window's content view. The C# resolves the window via Platform.GetCurrentWindow(); the port
// uses the app's key window (no WindowStateManager). SIMULATOR / UNBUNDLED-PROCESS NOTE (share.hpp):
// the spawned gtest process has no key window, so there is nothing to present from - the picker path
// is exercised only inside a real app; here, with no window, the request completes without presenting
// (the documented stand-in, the AppKit twin of browser.ios's no-view-controller path). The request
// validation already ran in the facade. Compiled as Objective-C++ with ARC for the apple backend.

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

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

        // The key window's content view (nil in the unbundled gtest process - see the header note).
        NSView* current_content_view()
        {
            NSWindow* const window = [NSApplication sharedApplication].keyWindow;
            return window != nil ? window.contentView : nil;
        }

        // PlatformShowRequestAsync: present the picker relative to the source bounds in the content
        // view, when a window exists. A no-op (no presentation) when there is no window.
        void show_request(const share_request_base& request, NSArray* items)
        {
            NSView* const view = current_content_view();
            if (view == nil || items.count == 0)
            {
                return;
            }
            const auto& bounds = request.presentation_source_bounds;
            // The C# flips Y to AppKit's bottom-left origin (view.Bounds.Height - rect.Bottom).
            const NSRect rect =
                NSMakeRect(bounds.x, view.bounds.size.height - (bounds.y + bounds.height), bounds.width, bounds.height);
            NSSharingServicePicker* const picker = [[NSSharingServicePicker alloc] initWithItems:items];
            [picker showRelativeToRect:rect ofView:view preferredEdge:NSRectEdgeMinY];
        }

        class apple_share final : public i_share
        {
        public:
            void request_async(const share_text_request& request, share_completion_callback on_complete) override
            {
                NSMutableArray* const items = [NSMutableArray array];
                if (!is_blank(request.title))
                {
                    [items addObject:to_ns_string(request.title)];
                }
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
                show_request(request, items);
                if (on_complete)
                {
                    on_complete();
                }
            }

            void request_async(const share_file_request& request, share_completion_callback on_complete) override
            {
                // PlatformRequestAsync(ShareFileRequest) routes to the multi-file path in C#.
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
                if (!is_blank(request.title))
                {
                    [items addObject:to_ns_string(request.title)];
                }
                for (const share_file& file : request.files)
                {
                    [items addObject:[NSURL fileURLWithPath:to_ns_string(file.full_path())]];
                }
                show_request(request, items);
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
            return std::make_shared<apple_share>();
        }
    } // namespace detail
} // namespace maui::application_model::data_transfer
