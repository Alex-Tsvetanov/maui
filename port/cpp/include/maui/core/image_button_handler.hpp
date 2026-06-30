#pragma once
// maui::core::image_button_handler  <=  Microsoft.Maui.Handlers.ImageButtonHandler
//
// The handler for a button that displays an image: the image pipeline (aspect + source through the
// handler-owned async loader) layered with the button surface (padding + stroke + the
// pressed/released/clicked inbound channel). Ported from ImageButtonHandler.cs (whose Mapper chains
// ImageHandler.Mapper and adds the IButtonStroke + Padding keys, and whose SourceLoader is its own
// ImageSourcePartLoader) + ImageButtonHandler.iOS.cs (UIButton + SetImage + the touch proxy).
//
// Because i_image_button re-declares (rather than derives) the i_image surface, the image map
// functions are this handler's own, keyed on i_image_button — the C# "chain ImageHandler.Mapper"
// collapses into listing the same four image keys here (documented with the contract's deviation).
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor + the cross-platform map_source routing
// are in image_button_handler.cpp; the platform recipe — create/connect/disconnect/map_*/the source
// primitives/measure — is per backend under src/platform/<backend>/image_button_handler.{cpp,mm}.
//
// image_button_platform is a single cross-platform struct: `native` holds the real backend view (an
// NSButton* on apple / a UIButton* on ios, retained in the .mm; unused headless); the value fields
// mirror every mapped property; the source mirrors match image_platform's (kind/file/loaded) so the
// headless tests observe the load; and the on_click/on_press/on_release hooks are the inbound channel
// (the button_platform convention).

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/aspect.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct image_button_platform : view_platform_base
    {
        image_button_platform() = default;
        ~image_button_platform() override; // backend-defined: releases the retained native button on apple/ios
        image_button_platform(const image_button_platform&) = delete;
        image_button_platform(image_button_platform&&) = delete;
        image_button_platform& operator=(const image_button_platform&) = delete;
        image_button_platform& operator=(image_button_platform&&) = delete;

        void* native = nullptr;
        // Headless mirrors of the image surface (the image_platform convention).
        aspect image_aspect = aspect::aspect_fit;
        std::string source_kind;
        std::string source_file;
        bool source_loaded = false;
        bool opaque = false;
        // Intrinsic size of the decoded file bitmap (the image_platform convention). The android partial
        // (wave 19) reports this from get_desired_size for the aspect-fit auto-size of an unconstrained
        // ImageButton (e.g. the Custom-Size dotnet_bot button). Apple/iOS measure via sizeThatFits and
        // headless returns {0,0}, so those backends leave these at 0; android is the only writer.
        double intrinsic_width = 0.0;
        double intrinsic_height = 0.0;
        // Headless mirror of IsAnimationPlaying (the image_platform convention; apple/ios drive native
        // GIF frame cycling instead). ImageButton pins the value false, so this stays false in practice.
        bool animation_playing = false;
        // Headless mirrors of the button surface (the button_platform convention).
        thickness padding;
        maui::graphics::color stroke_color;
        double stroke_thickness = 0;
        int corner_radius = 0;
        // Inbound channel hooks (wired by the platform partial; headless tests invoke them directly).
        move_only_function<void()> on_click;
        move_only_function<void()> on_press;
        move_only_function<void()> on_release;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSButton (defined in
        // src/platform/apple/image_button_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the four fundamental IView properties to the UIButton (defined in
        // src/platform/ios/image_button_handler.mm).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // BackgroundColor IS pushed: a system UIButton draws it as a per-state backgroundImage (a solid
        // fill becomes a 1×1 colored image; gradient/image defers to apply_background) — the ButtonHandler
        // recipe; plain backgroundColor is ignored by the button's own drawing.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the UIButton's layer (the shared
        // apply_and_store_clip; the handler's platform_arrange re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (wave 19): push the generic IView properties to the dev.mauicpp.MauiImageView
        // (an android.widget.ImageView subclass — the image rides setImageBitmap + setScaleType, the
        // chrome rides a GradientDrawable background), mirroring the android button partial. Each body
        // runs view_platform_base FIRST (the headless mirror must stay live for the VM-less pure-native
        // cross-platform suite on the emulator) then pushes to the real widget when a VM + app context
        // exist. Defined in src/platform/android/image_button_handler.cpp.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        // Background IS pushed: the ImageButton's solid BackgroundColor / brush fills the SHARED maui
        // GradientDrawable installed as the MauiImageView's background (the same drawable the stroke +
        // corner-radius map functions write to — ButtonExtensions' intertwined stroke recipe). A
        // gradient paint installs a GradientDrawable color ramp; a null paint clears OUR drawable. This
        // is what makes the page's green/purple-filled ImageButtons visible (cog.png is SVG-only and
        // never rasterizes, exactly as on iOS — the fill is the chrome).
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class image_button_handler : public view_handler<image_button_handler, i_image_button, image_button_platform>
    {
    public:
        image_button_handler();

        static property_mapper<i_image_button, image_button_handler>& mapper();
        static command_mapper<i_image_button, image_button_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native button's touches back to the virtual view.
        static std::unique_ptr<image_button_platform> create_platform_view();
        void on_connect_handler(image_button_platform& platform);
        static void on_disconnect_handler(image_button_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- the image surface (the ImageMapper chain, keyed on i_image_button) ----
        static void map_aspect(image_button_handler& handler, i_image_button& view);
        // Cross-platform source routing (image_button_handler.cpp): file fast-path synchronous, any
        // other source async via source_loader() with the identity recheck — image_handler::map_source's
        // twin over the image_button platform primitives.
        static void map_source(image_button_handler& handler, i_image_button& view);
        static void map_is_opaque(image_button_handler& handler, i_image_button& view);
        // IsAnimationPlaying → start/stop native multi-frame playback (mirrors image_handler::
        // map_is_animation_playing; headless mirrors the flag). Re-applied after a source load so a
        // freshly-decoded animated image starts playing — ImageButton pins the value to false (C#
        // IImageSourcePart.IsAnimationPlaying => false), but the re-push is part of the inherited
        // ImageHandler.MapSource pipeline (ImageHandler.iOS.cs:68 / .Android.cs:73) so it is faithful here.
        static void map_is_animation_playing(image_button_handler& handler, i_image_button& view);

        // ---- the button surface (ImageButtonHandler.Mapper's own keys) ----
        static void map_padding(image_button_handler& handler, i_image_button& view);
        static void map_stroke_color(image_button_handler& handler, i_image_button& view);
        static void map_stroke_thickness(image_button_handler& handler, i_image_button& view);
        static void map_corner_radius(image_button_handler& handler, i_image_button& view);

        // The handler-owned async image-source loader (C#'s SourceLoader / ImageSourcePartLoader).
        [[nodiscard]] image_source_loader& source_loader()
        {
            return source_loader_;
        }

    private:
        // Per-backend source primitives (the image_handler convention; defined in the platform partial).
        static void load_file_source_sync(image_button_platform& platform, const i_file_image_source& file_src);
        static void apply_loaded_result(image_button_platform& platform, const image_source_result& result);
        static void clear_source_native(image_button_platform& platform);
        static void configure_loader(image_source_loader& loader);

        image_source_loader source_loader_;
    };
} // namespace maui::core
