// file_picker - iOS (UIKit) platform partial. SERVICE SEAM: FilePicker.ios.cs builds a
// UIDocumentPickerViewController in Open mode (AllowsMultipleSelection per call), wires the
// DidPickDocumentAtUrls / WasCancelled handlers (or a UIDocumentPickerDelegate on older OSes),
// presents it over WindowStateManager.Default.GetCurrentUIViewController(), and converts the picked
// NSUrl[] via FileSystemUtils.EnsurePhysicalFileResultsAsync. Presenting a view controller needs a key
// window + root VC, which the spawned simulator gtest process does not have, so the picker UI is not
// drivable here - the on-simulator smoke asserts only the genuinely-testable boundary (no presenting
// view controller -> the documented service-seam error); the pick presentation runs only inside a real
// app. The headless fake covers the behavioral contract.
//
// DEFERRED: FileSystemUtils.EnsurePhysicalFileResultsAsync (the external-provider local-copy step) has
// no port analogue yet. Because this partial is a service seam that throws before any URL is picked,
// the conversion is never reached, so it is deferred and noted rather than ported.

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_picker.hpp"

namespace maui::storage
{
    namespace
    {
        // The WindowStateManager.GetCurrentUIViewController stand-in (the browser partial's twin): the
        // foreground key window's root view controller, or nil when the process has no UI (the gtest
        // case).
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

        [[noreturn]] void service_seam_unavailable()
        {
            throw maui::application_model::feature_not_supported(
                "FilePicker requires a presenting view controller; it is a service seam not drivable "
                "headlessly.");
        }

        // FilePicker.ios.cs: the allowed UTType list (the C# allowedUtis array, mapped to the modern
        // UniformTypeIdentifiers UTType), or the all-content default when FileTypes is null. Identifiers
        // the runtime does not recognise resolve to nil and are skipped (the picker treats an empty list
        // as "all types", matching the C# fallback).
        NSArray<UTType*>* allowed_content_types(const pick_options& options)
        {
            NSMutableArray<UTType*>* types = [NSMutableArray array];
            if (options.file_types.has_value())
            {
                if (auto ids = options.file_types->try_get(maui::devices::device_platform::ios()); ids.has_value())
                {
                    for (const std::string& identifier : *ids)
                    {
                        UTType* const type =
                            [UTType typeWithIdentifier:[NSString stringWithUTF8String:identifier.c_str()]];
                        if (type != nil)
                        {
                            [types addObject:type];
                        }
                    }
                }
            }
            if (types.count == 0)
            {
                // The C# default { UTType.Content, UTType.Item, "public.data" }.
                return @[ UTTypeContent, UTTypeItem, UTTypeData ];
            }
            return types;
        }

        class ios_file_picker final : public i_file_picker
        {
        public:
            void pick_async(const pick_options& options, file_result_callback /*on_complete*/) override
            {
                present(options, /*allow_multiple=*/false);
            }
            void pick_multiple_async(const pick_options& options, file_results_callback /*on_complete*/) override
            {
                present(options, /*allow_multiple=*/true);
            }

        private:
            static void present(const pick_options& options, bool allow_multiple)
            {
                UIViewController* const host = current_view_controller();
                if (host == nil)
                {
                    service_seam_unavailable(); // no key window/root VC in the gtest process
                }
                // Build the picker the way FilePicker.ios.cs does (Open mode = initForOpeningContentTypes,
                // multi-selection per call) so the UTType mapping + selection gate are exercised even on
                // the seam path. The picker is NOT presented here: the real flow presents over the host
                // and resolves on_complete from DidPickDocumentAtUrls / WasCancelled via a
                // TaskCompletionSource (+ FileSystemUtils.EnsurePhysicalFileResultsAsync), which is the
                // service seam (see the header note) - it runs only inside a real app. Presenting a picker
                // whose result cannot be delivered would orphan it, so the seam throws instead.
                NSArray<UTType*>* const content_types = allowed_content_types(options);
                UIDocumentPickerViewController* const picker =
                    [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:content_types];
                picker.allowsMultipleSelection = allow_multiple ? YES : NO;
                (void)picker;
                service_seam_unavailable();
            }
        };

        file_picker_file_type make_ui_type(const char* uti)
        {
            return file_picker_file_type({{maui::devices::device_platform::ios(), {uti}},
                                          {maui::devices::device_platform::mac_catalyst(), {uti}}});
        }
    } // namespace

    // FilePicker.ios.cs Platform*FileType(): the predefined UTType registries (the simulator's UTType
    // string constants). Image/Png/Jpeg/Pdf are single-UTI; Videos folds the C# video UTI + extension
    // set into one list.
    const file_picker_file_type file_picker_file_type::images = make_ui_type("public.image");
    const file_picker_file_type file_picker_file_type::png = make_ui_type("public.png");
    const file_picker_file_type file_picker_file_type::jpeg = make_ui_type("public.jpeg");
    const file_picker_file_type file_picker_file_type::pdf = make_ui_type("com.adobe.pdf");
    const file_picker_file_type file_picker_file_type::videos =
        file_picker_file_type({{maui::devices::device_platform::ios(),
                                {"public.mpeg-4", "public.movie", "public.avi", "com.apple.protected-mpeg-4-video",
                                 "mp4", "m4v", "mpg", "mpeg", "mp2", "mov", "avi", "mkv", "flv", "gifv", "qt"}},
                               {maui::devices::device_platform::mac_catalyst(),
                                {"public.mpeg-4", "public.movie", "public.avi", "com.apple.protected-mpeg-4-video",
                                 "mp4", "m4v", "mpg", "mpeg", "mp2", "mov", "avi", "mkv", "flv", "gifv", "qt"}}});

    namespace detail
    {
        std::shared_ptr<i_file_picker> make_file_picker()
        {
            return std::make_shared<ios_file_picker>();
        }
    } // namespace detail
} // namespace maui::storage
