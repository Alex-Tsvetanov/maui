// image_handler — Apple (AppKit / macOS) platform recipe (minimal: aspect only). The real-native twin of
// the headless partial: the managed platform view is an NSImageView (held, retained, in
// image_platform::native), and the cross-platform aspect maps to the view's imageScaling (+ imageAlignment).
// No image bytes are loaded this cut — the view simply exists and the scaling enum maps. Compiled as
// Objective-C++ with ARC only for the `apple` backend.
//
// Translated from ImageHandler.iOS.cs + AspectExtensions.cs (UIKit's UIViewContentMode):
//   AspectFit  → ScaleAspectFit   → NSImageScaleProportionallyUpOrDown
//   AspectFill → ScaleAspectFill   → NSImageScaleProportionallyUpOrDown + centered  [*]
//   Fill       → ScaleToFill       → NSImageScaleAxesIndependently
//   Center     → Center            → NSImageScaleNone + centered
// [*] AppKit's NSImageScaling has no exact aspect-fill-with-clipping mode (NSImageView does not clip an
//     up-scaled image the way UIView ScaleAspectFill does); proportional + centered is the closest
//     built-in approximation. This imperfect mapping is noted rather than worked around (a clipping
//     container would be a larger change, out of scope for this minimal cut).

#import <AppKit/AppKit.h>

#include <memory>

#include "maui/core/aspect.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/image_handler.hpp"
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
