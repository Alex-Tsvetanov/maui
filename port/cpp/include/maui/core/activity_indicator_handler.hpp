#pragma once
// maui::core::activity_indicator_handler  <=  Microsoft.Maui.Handlers.ActivityIndicatorHandler
//
// The handler for the indeterminate busy spinner (maui::controls::activity_indicator): IsRunning /
// Color flow virtual→native through the mapper (display-only — no inbound channel). Ported from
// ActivityIndicatorHandler.cs (cross-platform) + ActivityIndicatorHandler.iOS.cs /
// ActivityIndicatorExtensions.cs (the platform recipe; the AppKit backend translates the
// UIActivityIndicatorView recipe to a spinning-style NSProgressIndicator).
//
// As in C# on iOS/Android, the mapper REPLACES the generic Visibility mapping with MapIsRunning
// ("Visibility and IsRunning are dependent on each other, so we handle Visibility explicitly"): the
// spinner is visible-and-animating only while IsRunning && Visible. The Collapsed layout-constraint
// dance (Collapse/Inflate) is the same simplification as the other handlers — Hidden and Collapsed
// both hide the view (see button_handler's ios note).
//
// WINDOWS IS THE ODD ONE OUT: the C# Mapper only routes Visibility -> MapIsRunning under
// `#if __ANDROID__ || __IOS__ || MACCATALYST` — Windows keeps Visibility on the plain generic push,
// because ProgressRing.IsActive and UIElement.Visibility are independent there
// (ActivityIndicatorExtensions.Windows.cs's UpdateIsRunning is just `IsActive = IsRunning`). This
// shared cross-platform mapper() table (src/core/activity_indicator_handler.cpp) still redirects
// "visibility" to map_is_running unconditionally for every backend rather than branching on
// MAUI_PLATFORM_WINDOWS; src/platform/windows/activity_indicator_handler.cpp's map_is_running
// reproduces both native pushes independently from that one entry point instead — see its file-top
// deviation note 1 for the full writeup.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_activity_indicator.hpp"
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
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native spinner).
    struct activity_indicator_platform : view_platform_base
    {
        activity_indicator_platform() = default;
        ~activity_indicator_platform() override; // backend-defined: releases the retained native spinner
        activity_indicator_platform(const activity_indicator_platform&) = delete;
        activity_indicator_platform(activity_indicator_platform&&) = delete;
        activity_indicator_platform& operator=(const activity_indicator_platform&) = delete;
        activity_indicator_platform& operator=(activity_indicator_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple build pushes to `native` AND keeps
        // `is_running` as the observable animation state — NSProgressIndicator exposes no isAnimating
        // getter; iOS reads the real UIActivityIndicatorView.isAnimating instead). `hidden` (from
        // view_platform_base) mirrors the visibility half of UpdateIsRunning.
        bool is_running = false;
        maui::graphics::color color;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native ProgressRing via the shared
        // winui_visual_ops helpers (src/platform/windows/), exactly like picker_platform's identical
        // block. Selected by MAUI_PLATFORM_WINDOWS, which is PUBLIC on maui_core for that backend only —
        // so every TU of a given build sees exactly one backend's overrides and the class layout stays
        // ODR-consistent. ProgressRing IS a Control (unlike label_handler's bare TextBlock), so
        // IsEnabled/Background reach it directly with no wrapper/container needed.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSProgressIndicator (defined in
        // src/platform/apple/activity_indicator_handler.mm). NSProgressIndicator is not an NSControl,
        // so is_enabled keeps the base mirror. Same ODR note as button_platform.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
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
        // iOS backend: push the fundamental IView properties to the UIActivityIndicatorView (defined
        // in src/platform/ios/activity_indicator_handler.mm). Not a UIControl: is_enabled keeps the
        // base mirror; the remaining generic-IView pushes likewise (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // BackgroundColor / Background brush IS pushed to the UIActivityIndicatorView — the band behind the
        // spinner (the shared apply_background; MauiIosActivityIndicator.layoutSubviews keeps a gradient
        // fill sized to bounds).
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosActivityIndicator (UIActivityIndicatorView)'s layer (the
        // shared apply_and_store_clip; MauiIosActivityIndicator.layoutSubviews re-frames the mask to the live bounds,
        // the 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: BackgroundColor / Background brush IS pushed to the ProgressBar — the band
        // behind the spinner (ViewExtensions.UpdateBackground → View.setBackgroundColor / a GradientDrawable,
        // via the shared android apply_background). Defined in src/platform/android/activity_indicator_handler.cpp.
        // Twin of the iOS override above; without it the generic-IView map_background pushes to the
        // view_platform_base no-op mirror and the yellow BackgroundColor never reaches the real widget.
        // The remaining generic-IView pushes stay the base mirrors here (see the .cpp header note).
        void update_background(const maui::graphics::paint* value) override;
#endif
    };

    class activity_indicator_handler
        : public view_handler<activity_indicator_handler, i_activity_indicator, activity_indicator_platform>
    {
    public:
        activity_indicator_handler();

        // Shared mapper tables (cross-platform — defined in src/core/activity_indicator_handler.cpp).
        static property_mapper<i_activity_indicator, activity_indicator_handler>& mapper();
        static command_mapper<i_activity_indicator, activity_indicator_handler>& command_mapper();

        // Platform recipe (defined per backend). Display-only: no connect/disconnect hooks needed.
        static std::unique_ptr<activity_indicator_platform> create_platform_view();

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). map_is_running also carries the "visibility" key
        // (the C# Visibility → MapIsRunning override).
        static void map_is_running(activity_indicator_handler& handler, i_activity_indicator& view);
        static void map_color(activity_indicator_handler& handler, i_activity_indicator& view);
    };
} // namespace maui::core
