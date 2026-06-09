#pragma once
// maui::core::image_handler  <=  Microsoft.Maui.Handlers.ImageHandler (aspect + sources)
//
// The handler for an image view: it maps the scaling mode (aspect) onto a native NSImageView and loads the
// image source into that view. Ported from ImageHandler.cs (cross-platform) + ImageHandler.iOS.cs (the
// UIImageView ContentMode recipe, translated to AppKit's NSImageView) + ImageSourceExtensions.cs.
//
// map_source: a FILE source loads SYNCHRONOUSLY (a local file is cheap — [[NSImage alloc]
// initWithContentsOfFile:] / a headless path mirror, kept from the first cut). Any OTHER source (uri /
// stream) routes through the handler-owned image_source_loader (ASYNC: resolve the service, load, apply
// only if still the current source — the source-identity recheck — then complete). This mirrors C#'s
// MapSource being a fire-and-forget async load via the SourceLoader, with the file fast-path as the
// only synchronous shortcut. DEFERRED: font image sources, disk caching + CacheValidity, resolution
// reload, the full DI service-provider (see image_source_loader.hpp).
//
// Partial-class split (PROFILE §5): the mapper TABLE + ctor are cross-platform (image_handler.cpp); the
// platform recipe — create / map_aspect / map_source / measure — is per backend under
// src/platform/<backend>/image_handler.{cpp,mm}. Only one backend is linked.
//
// image_platform is a single cross-platform struct: `native` holds the real backend view (an NSImageView*
// on Apple, retained in the .mm; unused headless), `image_aspect` mirrors the mapped scaling mode and
// `source_kind`/`source_file`/`source_loaded` mirror the resolved source for the headless tests
// (source_kind is "file"/"uri"/"stream"; source_file is the path/uri/"<bytes:N>"; the Apple build loads
// into `native` instead). It derives view_platform_base (the shared ViewMapper face) so the generic IView
// properties (Visibility/Opacity/IsEnabled/AutomationId) map onto the NSImageView too.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/aspect.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/image_source_loader.hpp"
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
        // Headless mirror of the resolved source: the kind ("file"/"uri"/"stream"/"font"), the resolved
        // path/uri/"<bytes:N>"/glyph, and whether a source is currently loaded. map_source (file fast-path)
        // and the async loader's apply set these; a null/empty source clears them (the Apple build sets the
        // real NSImageView.image instead). See src/platform/headless/image_handler.cpp.
        std::string source_kind;
        std::string source_file;
        bool source_loaded = false;
        // Headless mirrors of IsOpaque / IsAnimationPlaying (Apple pushes these to the NSImageView's layer /
        // animation state instead). is_animation_playing's native multi-frame animation is a documented
        // deviation — only the flag is stored.
        bool opaque = false;
        bool animation_playing = false;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSImageView (defined in
        // src/platform/apple/image_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the NSImageView (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate.
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
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

        // Property map functions (platform recipe).
        static void map_aspect(image_handler& handler, i_image& view);
        // Load view.source() into the native view. A FILE source loads synchronously (file fast-path,
        // headless mirrors the path); any other source (uri/stream/font) routes through source_loader()
        // async (apply only if still current). A null/empty source clears the image. The loader pushes the
        // in-flight loading state back via view.update_is_loading. See the .mm/.cpp twins.
        static void map_source(image_handler& handler, i_image& view);
        // IsOpaque → the native view's opacity hint (Apple: layer.opaque); headless mirrors the flag.
        static void map_is_opaque(image_handler& handler, i_image& view);
        // IsAnimationPlaying → start/stop the native animation (Apple: NSImageView animates / stops);
        // headless mirrors the flag. The multi-frame GIF decode is a documented deviation.
        static void map_is_animation_playing(image_handler& handler, i_image& view);

        // The handler-owned async image-source loader (C#'s SourceLoader). Tests inject a dispatcher here
        // (and pump it) to drive the async load deterministically; the apple recipe leaves it inline.
        [[nodiscard]] image_source_loader& source_loader()
        {
            return source_loader_;
        }

    private:
        // Per-backend source primitives map_source dispatches to (the routing — file fast-path vs the
        // async loader — lives once in the cross-platform map_source; only these touch the native view /
        // headless mirror). Defined in src/platform/<backend>/image_handler.{cpp,mm}.
        static void load_file_source_sync(image_platform& platform, const i_file_image_source& file_src);
        static void apply_loaded_result(image_platform& platform, const image_source_result& result);
        static void clear_source_native(image_platform& platform);

        image_source_loader source_loader_;
    };
} // namespace maui::core
