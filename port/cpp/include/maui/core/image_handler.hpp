#pragma once
// maui::core::image_handler  <=  Microsoft.Maui.Handlers.ImageHandler (aspect + sources)
//
// The handler for an image view: it maps the scaling mode (aspect) onto a native NSImageView and loads the
// image source into that view. Ported from ImageHandler.cs (cross-platform) + ImageHandler.iOS.cs (the
// UIImageView ContentMode recipe, translated to AppKit's NSImageView) + ImageSourceExtensions.cs.
//
// map_source: a FILE source loads SYNCHRONOUSLY (a local file is cheap — [[NSImage alloc]
// initWithContentsOfFile:] / a headless path mirror). Any OTHER source (uri / stream / font) routes through
// the handler-owned image_source_loader (ASYNC: resolve the service, load, apply only if still the current
// source — the source-identity recheck — then complete). This mirrors C#'s MapSource being a fire-and-forget
// async load via the SourceLoader, with the file fast-path as the only synchronous shortcut. The uri path
// is backed by the loader's two-layer cache (in-memory TTL + on-disk, both gated by CacheValidity) and, on
// apple, a real NSURLSession async fetch — see configure_loader + image_source_loader.hpp. DEFERRED: the
// full DI service-provider (the loader resolves against the built-in registry instead).
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

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/aspect.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/move_only_function.hpp"
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
        // Intrinsic size (in framework points) of the last decoded bitmap, or {0,0} when nothing is
        // loaded. The Android backend records the BitmapFactory-decoded bitmap's width/height here so
        // image_handler::get_desired_size can aspect-fit the real content size (the iOS SizeThatFitsImage
        // analog; a {0,0} measure collapses an auto-sized Image to nothing — the documented gap the
        // intrinsic measure closes). Headless leaves it {0,0} (no decode); Apple measures the NSImage/
        // UIImage directly instead, so this mirror is unused there.
        double intrinsic_width = 0.0;
        double intrinsic_height = 0.0;
        // Headless mirrors of IsOpaque / IsAnimationPlaying (Apple pushes these to the NSImageView's layer /
        // animation state instead — on apple IsAnimationPlaying drives native GIF frame cycling).
        bool opaque = false;
        bool animation_playing = false;
        // Non-owning borrow of the last-mapped clip geometry (VisualElement.Clip / IView.Clip), refreshed on
        // every update_clip incl. the null clear. The Android backend re-resolves it against the LIVE bounds
        // from platform_arrange (the iOS reapply_clip analog — the clip path is bounds-dependent, so a resize
        // must rebuild + reinstall it). Null when no clip is set. The control owns the shape's lifetime
        // (controls/view.hpp holds it by shared_ptr); this is only the const* borrow, exactly as the iOS
        // store_clip_shape stash. Unused on headless/Apple (Apple stashes via an associated object instead).
        const maui::graphics::i_shape* clip_shape = nullptr;
        // Windows-only inbound hook: fired once BitmapImage decoding completes — either by the native
        // ImageOpened subscription (image_handler.cpp's on_connect_handler) or, for an already-decoded
        // source, by the belt-and-braces check in load_file_source_sync/apply_loaded_result (see that
        // file's header note 2 for why both paths exist). Indirected through the platform struct rather
        // than a direct handler `this` capture — the button_platform on_click convention — so (a) the
        // static per-backend source primitives (which have no `this`) can invoke it too, and (b) a token
        // revoke that is somehow skipped resolves to a safe no-op instead of touching a dangling handler.
        // Unused (never invoked) on every other backend.
        maui::core::move_only_function<void()> on_image_opened;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: the ImageOpened event-registration token on_connect_handler produces, so
        // on_disconnect_handler (and ~image_platform, which calls it directly so a handler torn down
        // without a disconnect does not leave the lambda firing into freed memory) can revoke EXACTLY
        // what it registered — the button_platform click_token convention.
        std::int64_t image_opened_token = 0;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native Image element via the shared
        // winui_visual_ops helpers (src/platform/windows/image_handler.cpp). Selected by
        // MAUI_PLATFORM_WINDOWS, which is PUBLIC on maui_core for that backend only - so every TU of a
        // given build sees exactly one backend's overrides and the class layout stays ODR-consistent.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // NOW LANDS: image_handler.cpp's create_platform_view wraps the Image in a Border host (the
        // container ImageHandler.Windows.NeedsContainer/SetupContainer introduces conditionally on
        // Background/AspectFill — see that file's header for why this port wraps unconditionally, like
        // label_handler.cpp), and apply_background's existing three-way try_as already fills a Border.
        void update_background(const maui::graphics::paint* value) override;
#endif

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

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out): push the four fundamental IView properties to the UIImageView
        // (defined in src/platform/ios/image_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors until the shared ios view/visual/semantics op helpers land (the
        // coordinator's retrofit; see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the UIImageView's layer via the shared
        // apply_background — e.g. the clip page's gray fill behind the photo.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the UIImageView's layer (the shared
        // apply_and_store_clip; the handler's platform_arrange re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (wave 11): the ONE generic-IView push wired for the image so far is Clip
        // (VisualElement.Clip / IView.Clip). The C# ViewExtensions.UpdateClip masks a WrapperView; the port
        // has no per-control WrapperView, so the clip rides the custom dev.mauicpp.MauiImageView, which
        // clips its own onDraw to a native android.graphics.Path the handler installs via setClipPath (the
        // CAShapeLayer-mask analog). update_clip builds that Path from the shape resolved against the view's
        // CURRENT bounds and stashes the borrow in clip_shape so platform_arrange can rebuild + reinstall it
        // on a resize (the iOS reapply_clip analog — the geometry is bounds-dependent). A null clip clears
        // the path. The remaining generic-IView pushes keep the view_platform_base mirrors (the image_platform
        // android override block is otherwise out of scope — see the deviations note in image_handler.cpp).
        // This wave scopes clip to images only (the target pages clip images); generalizing the same
        // native-Path-clip to any view's VisualElement.Clip (a shared android clip op + a custom-View host
        // per control) is future work. // TODO: a shared android clip op once more controls need a native clip.
        void update_clip(const maui::graphics::i_shape* value) override;
        // Background IS pushed (the clip/clip_gallery pages frame each image with a LightGray box —
        // VisualElement.BackgroundColor). C# ViewHandler.UpdateBackground sets the View's background to the
        // paint's drawable; for a solid_paint the faithful plain-View analog is View.setBackgroundColor(argb)
        // (the same primitive the collection_view cell background uses). A non-solid/absent paint clears it
        // (setBackgroundColor(TRANSPARENT)), keeping the base mirror. The remaining generic-IView pushes still
        // keep only the view_platform_base mirrors. // TODO: gradient/tiled brushes when a shared android
        // paint→drawable bridge lands.
        void update_background(const maui::graphics::paint* value) override;
        // Opacity IS pushed (ViewExtensions.UpdateOpacity: View.Alpha = (float)opacity) — the image page's
        // Opacity=0.5 rows fade the ImageView. Base mirror first (the VM-less suite observes it), then the
        // native setAlpha. The remaining generic-IView pushes keep only the view_platform_base mirrors.
        void update_opacity(double value) override;
#endif
    };

    class image_handler : public view_handler<image_handler, i_image, image_platform>
    {
    public:
        image_handler();

        static property_mapper<i_image, image_handler>& mapper();
        static command_mapper<i_image, image_handler>& command_mapper();

        static std::unique_ptr<image_platform> create_platform_view();

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend ONLY: subscribe/unsubscribe Microsoft.UI.Xaml.Controls.Image.ImageOpened — the
        // consumer half of the async-decode fix (this file's header note 2; image_handler.cpp's own
        // header). No other backend decodes asynchronously, so no other backend declares these — the
        // view_handler.hpp `if constexpr (requires …)` detection simply finds nothing on those builds and
        // skips the call, matching the pre-existing (no-op) behavior there exactly.
        void on_connect_handler(image_platform& platform);
        static void on_disconnect_handler(image_platform& platform);
#endif

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
        // IsAnimationPlaying → start/stop native multi-frame (GIF) playback (Apple: NSImageView.animates
        // cycles an animated NSImage's frames; stops on the current frame when false); headless mirrors the
        // flag. Re-applied after a source load so a freshly-decoded animated image starts playing.
        static void map_is_animation_playing(image_handler& handler, i_image& view);

        // The handler-owned async image-source loader (C#'s SourceLoader). Tests inject a dispatcher here
        // (and pump it) to drive the async load deterministically; the apple recipe leaves it inline.
        [[nodiscard]] image_source_loader& source_loader()
        {
            return source_loader_;
        }

        // C# ImageHandler.OnWindowChanged: when the loaded image was resolution-dependent (a font image)
        // AND the display density has changed (e.g. the view moved to an @3x screen), re-issue the source
        // so it re-rasterizes at the new density. The current density comes from the per-backend
        // query_display_scale() seam; the loader's requires_reload() compares it against the load-time
        // density. A no-op when the last result was not resolution-dependent (file/uri/stream). Drives the
        // SAME map_source the property change does. Tests call this directly after changing the scale seam.
        void on_window_changed();

        // Push the current display density into the loader (C#'s uiContext.GetDisplayDensity(), captured at
        // complete_load into CurrentResolution). Called from map_source before each load and from the
        // per-backend window-change hook; tests set the scale directly to drive requires_reload
        // deterministically. The per-backend query_display_scale() reads the real screen DPI (UIScreen /
        // NSScreen); headless reports 1.0 (no display).
        void refresh_display_scale();

    private:
        // Per-backend screen-DPI seam: the current display density (apple: the view's window backing scale;
        // ios: the trait-collection displayScale; headless: 1.0). Defined in the per-backend partial.
        [[nodiscard]] float query_display_scale() const;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend ONLY: mirrors ImageHandler.Windows.cs's private UpdatePlatformMaxConstraints
        // (oracle src/Core/src/Handlers/Image/ImageHandler.Windows.cs:234-261). Caps the native Image's own
        // MaxWidth/MaxHeight -- NOT the Border host's Width/Height (this file's header CONTAINER note: the
        // host is what get_desired_size/platform_arrange size and arrange; the child Image's own Width/
        // Height stay untouched Auto/NaN forever, permanently -- MaxWidth/MaxHeight is a DIFFERENT WinUI
        // property pair from Width/Height, so capping it does not disturb that or reintroduce the 0x0
        // measure bug commit 0165f5e6b9 fixed). Called from OnImageOpened (image_handler.cpp's
        // on_connect_handler lambda), exactly where the oracle calls it (:217-227), once the decoded
        // PixelWidth/PixelHeight are available. No other backend declares this (only WinUI decodes
        // asynchronously and exposes a native MaxWidth/MaxHeight this port bothers to set).
        void update_platform_max_constraints();
#endif

        // Per-backend source primitives map_source dispatches to (the routing — file fast-path vs the
        // async loader — lives once in the cross-platform map_source; only these touch the native view /
        // headless mirror). Defined in src/platform/<backend>/image_handler.{cpp,mm}.
        static void load_file_source_sync(image_platform& platform, const i_file_image_source& file_src);
        static void apply_loaded_result(image_platform& platform, const image_source_result& result);
        static void clear_source_native(image_platform& platform);
        // Per-backend loader wiring, called once from the constructor: apple installs the NSURLSession async
        // uri fetch + the NSCachesDirectory disk-cache directory; headless leaves the loader on its defaults
        // (synchronous read_uri_bytes, disk layer off). Defined in src/platform/<backend>/image_handler.*.
        static void configure_loader(image_source_loader& loader);

        image_source_loader source_loader_;
    };
} // namespace maui::core
