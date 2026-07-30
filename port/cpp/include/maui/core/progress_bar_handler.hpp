#pragma once
// maui::core::progress_bar_handler  <=  Microsoft.Maui.Handlers.ProgressBarHandler
//
// The handler for the determinate progress bar (maui::controls::progress_bar): Progress /
// ProgressColor flow virtual→native through the mapper (display-only — no inbound channel). Ported
// from ProgressBarHandler.cs (cross-platform) + ProgressBarHandler.iOS.cs / ProgressBarExtensions.cs
// (the platform recipe; the AppKit backend translates the UIProgressView recipe to a determinate-bar
// NSProgressIndicator).
//
// The iOS FlowDirection mapper override IS ported (map_flow_direction): the bar-specific
// UISemanticContentAttribute recipe with the MatchParent → parent-IView fallback + the iOS-26 subview
// re-application; the apple twin maps it to NSView's layout direction with the same parent fallback.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_progress.hpp"
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
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native bar).
    struct progress_bar_platform : view_platform_base
    {
        progress_bar_platform() = default;
        ~progress_bar_platform() override; // backend-defined: releases the retained native bar on Apple/iOS
        progress_bar_platform(const progress_bar_platform&) = delete;
        progress_bar_platform(progress_bar_platform&&) = delete;
        progress_bar_platform& operator=(const progress_bar_platform&) = delete;
        progress_bar_platform& operator=(progress_bar_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds push to `native` instead;
        // progress_color is also the AppKit observable mirror — NSProgressIndicator has no fill-color
        // API, see src/platform/apple/progress_bar_handler.mm).
        double progress = 0;
        maui::graphics::color progress_color;
        // The RESOLVED flow direction the MapFlowDirection recipe computed (after the MatchParent →
        // parent-IView fallback). Headless records it as the observable mirror; the Apple/iOS builds push
        // it to the native bar (NSView layout direction / UISemanticContentAttribute) AND mirror it here.
        maui::core::flow_direction resolved_flow_direction = maui::core::flow_direction::match_parent;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native
        // Microsoft.UI.Xaml.Controls.ProgressBar via the shared winui_visual_ops helpers
        // (src/platform/windows/), exactly like slider_platform's / activity_indicator_platform's
        // identical block. Selected by MAUI_PLATFORM_WINDOWS, which is PUBLIC on maui_core for that
        // backend only — so every TU of a given build sees exactly one backend's overrides and the class
        // layout stays ODR-consistent. ProgressBar IS a Control (RangeBase : Control), unlike
        // label_handler's bare TextBlock, so IsEnabled/Background reach it directly with no
        // wrapper/container needed. No new DATA fields are added here (display-only handler, no native
        // events to track), so this addition does not change the struct's layout on non-Windows builds.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSProgressIndicator (defined in
        // src/platform/apple/progress_bar_handler.mm). Same ODR note as button_platform. An
        // NSProgressIndicator is not an NSControl, so is_enabled keeps the base mirror (the C# bar has
        // no enabled state on iOS either).
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
        // iOS backend: push the fundamental IView properties to the UIProgressView (defined in
        // src/platform/ios/progress_bar_handler.mm). UIProgressView is not a UIControl, so is_enabled
        // keeps the base mirror; the remaining generic-IView pushes likewise (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the UIProgressView's layer via the shared
        // apply_background (mirroring the apple backend) — behind the track.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the UIProgressView's layer (the shared
        // apply_and_store_clip; the handler's platform_arrange re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (M-android fan-out): push the generic IView properties to the real
        // android.widget.ProgressBar over JNI (defined in src/platform/android/progress_bar_handler.cpp).
        // Each override calls the view_platform_base body FIRST (the VM-less cross-platform suite observes
        // the headless mirror) then pushes to the widget when one exists; transform / flow-direction /
        // semantics route through the shared android ops. IsEnabled, Shadow, Clip, and InputTransparent
        // keep ONLY the base mirror (no plain-ProgressBar analog), as the button partial documents.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class progress_bar_handler : public view_handler<progress_bar_handler, i_progress, progress_bar_platform>
    {
    public:
        progress_bar_handler();

        // Shared mapper tables (cross-platform — defined in src/core/progress_bar_handler.cpp).
        static property_mapper<i_progress, progress_bar_handler>& mapper();
        static command_mapper<i_progress, progress_bar_handler>& command_mapper();

        // Platform recipe (defined per backend). Display-only: no connect/disconnect hooks needed.
        static std::unique_ptr<progress_bar_platform> create_platform_view();

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe).
        static void map_progress(progress_bar_handler& handler, i_progress& view);
        static void map_progress_color(progress_bar_handler& handler, i_progress& view);
        // ProgressBarHandler.MapFlowDirection (iOS): overrides the shared view_mapper's generic flow push
        // with the bar-specific UISemanticContentAttribute recipe — Force{Left,Right}ToLeft from
        // FlowDirection, MatchParent falling back to the parent IView's FlowDirection, and (iOS-26) the
        // attribute re-applied to each internal subview. The apple twin maps it to NSView's layout
        // direction with the same parent fallback. Keyed on "flow_direction" in the handler mapper.
        static void map_flow_direction(progress_bar_handler& handler, i_progress& view);

        // ProgressBarHandler.GetSemanticContentAttribute / GetParentSemanticContentAttribute, collapsed:
        // the view's own FlowDirection, or — when MatchParent — the parent IView's FlowDirection (else
        // MatchParent when there is no IView parent). Cross-platform (progress_bar_handler.cpp); both the
        // Apple and iOS map_flow_direction call it to get the resolved direction to apply natively.
        [[nodiscard]] static maui::core::flow_direction resolved_flow_direction(const i_progress& view);
    };
} // namespace maui::core
