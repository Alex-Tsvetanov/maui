#pragma once
// maui::core::slider_handler  <=  Microsoft.Maui.Handlers.SliderHandler
//
// The handler for the linear-value input (maui::controls::slider): Minimum / Maximum / Value and the
// three colors flow virtual→native through the mapper; the native value change flows back through
// i_range::set_value and the native touch down/up through send_drag_started/send_drag_completed.
// Ported from SliderHandler.cs (cross-platform) + SliderHandler.iOS.cs / SliderExtensions.cs (the
// platform recipe; the AppKit backend translates the UISlider recipe to NSSlider).
//
// Same partial-class split + single cross-platform slider_platform struct as button_handler.
//
// ThumbImageSource (the async image-service fetch — see i_slider.hpp) and the iOS-specific UpdateOnTap
// platform configuration (the tap-to-set gesture from Slider.iOS.cs) are BOTH ported: ThumbImageSource
// flows through the handler-owned image_source_loader (the same seam as image_handler), and UpdateOnTap
// rides the i_ios_slider_specifics side contract (the W2-24 platform-configuration pattern).

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_slider.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native slider).
    struct slider_platform : view_platform_base
    {
        slider_platform() = default;
        ~slider_platform() override; // backend-defined: releases the retained native slider on Apple/iOS
        slider_platform(const slider_platform&) = delete;
        slider_platform(slider_platform&&) = delete;
        slider_platform& operator=(const slider_platform&) = delete;
        slider_platform& operator=(slider_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds push to `native` instead). For
        // the headless backend `value` doubles as the NATIVE thumb position: a test simulates a user
        // drag by setting it and invoking on_value_changed (the UISlider.ValueChanged analog), with
        // on_drag_started/on_drag_completed standing in for TouchDown / TouchUpInside|Outside.
        double minimum = 0;
        double maximum = 1;
        double value = 0;
        maui::graphics::color minimum_track_color;
        maui::graphics::color maximum_track_color;
        maui::graphics::color thumb_color;
        move_only_function<void()> on_value_changed;
        move_only_function<void()> on_drag_started;
        move_only_function<void()> on_drag_completed;
        // Headless mirrors of the ThumbImageSource fetch (whether a thumb image is currently set — the
        // Apple/iOS builds set the native thumb image instead) and the UpdateOnTap platform configuration
        // (whether the tap-to-set gesture is installed; the native builds attach/remove a real recognizer).
        bool thumb_image_set = false;
        bool update_on_tap = false;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSSlider (defined in
        // src/platform/apple/slider_handler.mm). Same ODR note as button_platform.
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
        // iOS backend: push the four fundamental IView properties to the UISlider (defined in
        // src/platform/ios/slider_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors, matching the other ios platform structs (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // BackgroundColor / Background brush IS pushed to the UISlider — a band behind the track (the shared
        // apply_background; MauiIosSlider.layoutSubviews keeps a gradient/image fill sized to bounds).
        void update_background(const maui::graphics::paint* value) override;
#endif
    };

    class slider_handler : public view_handler<slider_handler, i_slider, slider_platform>
    {
    public:
        slider_handler();

        // Shared mapper tables (cross-platform — defined in src/core/slider_handler.cpp). `mapper`
        // chains the shared view_mapper, mirroring SliderHandler.Mapper over ViewHandler.ViewMapper.
        static property_mapper<i_slider, slider_handler>& mapper();
        static command_mapper<i_slider, slider_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/slider_handler.{cpp,mm}).
        static std::unique_ptr<slider_platform> create_platform_view();
        void on_connect_handler(slider_platform& platform);
        static void on_disconnect_handler(slider_platform& platform);

        // Instance-level disconnect (C# SliderHandler.DisconnectHandler teardown): the static
        // on_disconnect_handler(slider_platform&) can't reach the member thumb_image_loader_, so cancel any
        // in-flight thumb-image load HERE first — a queued (dispatcher-pending) apply closure captures the
        // platform by raw pointer, and the base disconnect destroys that platform. update_source(nullptr,
        // nullptr) cancels the load's token so the queued apply is gated out instead of dereferencing the
        // freed platform (the UAF the review found). Then delegate to the base teardown. (C# is GC-safe
        // because it captures the managed UISlider; the port must cancel explicitly.)
        void disconnect_handler() override;

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe).
        static void map_minimum(slider_handler& handler, i_slider& view);
        static void map_maximum(slider_handler& handler, i_slider& view);
        static void map_value(slider_handler& handler, i_slider& view);
        static void map_minimum_track_color(slider_handler& handler, i_slider& view);
        static void map_maximum_track_color(slider_handler& handler, i_slider& view);
        static void map_thumb_color(slider_handler& handler, i_slider& view);
        // SliderHandler.MapThumbImageSource → SliderExtensions.UpdateThumbImageSourceAsync: resolve the
        // ThumbImageSource through the handler-owned image_source_loader and swap the native thumb image
        // (clearing the thumb tint while an image is set, exactly like C#). A null source clears it and
        // re-applies the thumb color. Keyed on "thumb_image_source"; map_thumb_color also re-runs it.
        static void map_thumb_image_source(slider_handler& handler, i_slider& view);
        // Slider.MapUpdateOnTap (the iOSSpecific UpdateOnTap remap): install/remove the tap-to-set gesture
        // on the native slider based on i_ios_slider_specifics::update_on_tap(). Keyed on the namespaced
        // platform-spec key "ios.Slider.UpdateOnTap" so a knob change re-runs it.
        static void map_update_on_tap(slider_handler& handler, i_slider& view);

        // The handler-owned async image-source loader (C#'s thumb-image fetch path). Tests inject a
        // dispatcher (and pump it) to drive the load deterministically; the apple recipe leaves it inline.
        [[nodiscard]] image_source_loader& thumb_image_loader()
        {
            return thumb_image_loader_;
        }

    private:
        // Per-backend primitives map_thumb_image_source routes to: apply the decoded image to the native
        // thumb, or clear it (and re-apply the thumb color). Defined in src/platform/<backend>/slider_handler.*.
        // `view` is passed to apply_thumb_image so the iOS recipe can tint the image with the ThumbColor when
        // both are set (SliderExtensions.cs ApplyTintColor) — mirroring clear_thumb_image's view parameter.
        static void apply_thumb_image(slider_platform& platform, i_slider& view, const image_source_result& result);
        static void clear_thumb_image(slider_platform& platform, i_slider& view);
        // Per-backend loader wiring (apple/ios: the NSURLSession async fetch + the cache directory; headless
        // leaves the loader on its synchronous defaults). Called once from the constructor.
        static void configure_thumb_loader(image_source_loader& loader);

        image_source_loader thumb_image_loader_;
    };
} // namespace maui::core
