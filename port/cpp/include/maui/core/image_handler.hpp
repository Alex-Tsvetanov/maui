#pragma once
// maui::core::image_handler  <=  Microsoft.Maui.Handlers.ImageHandler (minimal: aspect only)
//
// The handler for an image view — a MINIMAL cut that maps ONLY the scaling mode (aspect) onto a native
// NSImageView. No image bytes are loaded this cut (the async source subsystem is deferred); the view
// simply exists and the aspect enum maps to AppKit's imageScaling (+ imageAlignment). Ported from
// ImageHandler.cs (cross-platform) + ImageHandler.iOS.cs (the UIImageView ContentMode recipe, translated
// to AppKit's NSImageView).
//
// Partial-class split (PROFILE §5): the mapper TABLE + ctor are cross-platform (image_handler.cpp); the
// platform recipe — create / map_aspect / measure — is per backend under
// src/platform/<backend>/image_handler.{cpp,mm}. Only one backend is linked.
//
// image_platform is a single cross-platform struct: `native` holds the real backend view (an NSImageView*
// on Apple, retained in the .mm; unused headless), `image_aspect` mirrors the mapped scaling mode for the
// headless tests. It derives view_platform_base (the shared ViewMapper face) so the generic IView
// properties (Visibility/Opacity/IsEnabled/AutomationId) map onto the NSImageView too.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/aspect.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct image_platform : view_platform_base
    {
        image_platform() = default;
        ~image_platform() override; // backend-defined: releases the retained native image view on Apple
        image_platform(const image_platform&) = delete;
        image_platform(image_platform&&) = delete;
        image_platform& operator=(const image_platform&) = delete;
        image_platform& operator=(image_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of the mapped aspect (the Apple build writes to `native` instead).
        aspect image_aspect = aspect::aspect_fit;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSImageView (defined in
        // src/platform/apple/image_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class image_handler : public view_handler<image_handler, i_image, image_platform>
    {
    public:
        image_handler();

        static property_mapper<i_image, image_handler>& mapper();
        static command_mapper<i_image, image_handler>& command_mapper();

        static std::unique_ptr<image_platform> create_platform_view();

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map function (platform recipe).
        static void map_aspect(image_handler& handler, i_image& view);
    };
} // namespace maui::core
