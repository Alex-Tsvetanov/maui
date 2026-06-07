// image_handler — Apple (AppKit / macOS) platform recipe (aspect + file source). The real-native twin of
// the headless partial: the managed platform view is an NSImageView (held, retained, in
// image_platform::native); the cross-platform aspect maps to the view's imageScaling (+ imageAlignment),
// and a file source loads SYNCHRONOUSLY into the view's image. Compiled as Objective-C++ with ARC only for
// the `apple` backend.
//
// aspect — translated from ImageHandler.iOS.cs + AspectExtensions.cs (UIKit's UIViewContentMode):
//   AspectFit  → ScaleAspectFit   → NSImageScaleProportionallyUpOrDown
//   AspectFill → ScaleAspectFill   → NSImageScaleProportionallyUpOrDown + centered  [*]
//   Fill       → ScaleToFill       → NSImageScaleAxesIndependently
//   Center     → Center            → NSImageScaleNone + centered
// [*] AppKit's NSImageScaling has no exact aspect-fill-with-clipping mode (NSImageView does not clip an
//     up-scaled image the way UIView ScaleAspectFill does); proportional + centered is the closest
//     built-in approximation. This imperfect mapping is noted rather than worked around (a clipping
//     container would be a larger change, out of scope for this minimal cut).
//
// source — translated from ImageHandler.iOS.cs MapSource + ImageSourceExtensions.cs GetPlatformImage
// (UIImage.FromBundle(name) ?? UIImage.FromFile(file)). FIRST CUT: a file source loads SYNCHRONOUSLY via
// [[NSImage alloc] initWithContentsOfFile:] (AppKit's NSImage has no FromBundle split — the file path is
// loaded directly). A null/empty source clears imageView.image. DEFERRED: C#'s async fire-and-forget
// loader + cancellation, the IImageSourceService/service-provider seam, the non-file source kinds
// (uri/stream/font), and caching.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/aspect.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    NSImageView* as_image_view(void* native)
    {
        return (__bridge NSImageView*)native;
    }

    NSImageScaling to_ns_image_scaling(maui::core::aspect value)
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
            case maui::core::aspect::aspect_fill:
                return NSImageScaleProportionallyUpOrDown;
            case maui::core::aspect::fill:
                return NSImageScaleAxesIndependently;
            case maui::core::aspect::center:
                return NSImageScaleNone;
        }
        return NSImageScaleProportionallyUpOrDown;
    }

    // Synchronous file load: AppKit's [[NSImage alloc] initWithContentsOfFile:] (returns nil if the file is
    // missing or not a decodable image). The C# original tries FromBundle first; AppKit has no equivalent
    // path split, so the file path is loaded directly.
    NSImage* load_image_from_file(std::string_view path)
    {
        const std::string file(path);
        NSString* const ns_path = [NSString stringWithUTF8String:file.c_str()];
        if (ns_path == nil)
        {
            return nil;
        }
        return [[NSImage alloc] initWithContentsOfFile:ns_path];
    }
} // namespace

namespace maui::core
{
    image_platform::~image_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void image_platform::update_visibility(maui::core::visibility value)
    {
        as_image_view(native).hidden = value != maui::core::visibility::visible;
    }

    void image_platform::update_opacity(double value)
    {
        as_image_view(native).alphaValue = value;
    }

    void image_platform::update_is_enabled(bool value)
    {
        [as_image_view(native) setEnabled:static_cast<BOOL>(value)];
    }

    void image_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_image_view(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
        NSImageView* const view = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        platform->native = (__bridge_retained void*)view; // the void* slot owns one reference
        return platform;
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSImageView* const image_view = as_image_view(platform->native);
        image_view.imageScaling = to_ns_image_scaling(view.aspect());
        // Keep the image centered for every mode (NSImageView's own default), so Center and the
        // aspect-* approximations sit in the middle of the view rather than a corner.
        image_view.imageAlignment = NSImageAlignCenter;
    }

    // Synchronous file load (see the file header). A null/empty source clears the image; otherwise the file
    // source's path is loaded via NSImage. Only i_file_image_source is handled this cut (the other source
    // kinds are deferred — they also clear, rather than guess). A failed load (nil) leaves the image cleared.
    void image_handler::map_source(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSImageView* const image_view = as_image_view(platform->native);

        const i_image_source* const src = view.source();
        if (src == nullptr || src->is_empty())
        {
            image_view.image = nil;
            return;
        }

        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            image_view.image = load_image_from_file(file_src->file()); // nil on a failed load (clears it)
            return;
        }

        image_view.image = nil; // deferred source kind: clear rather than guess
    }

    maui::graphics::size image_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // No image bytes are loaded this cut, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_image_view(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
