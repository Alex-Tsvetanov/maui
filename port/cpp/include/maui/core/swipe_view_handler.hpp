#pragma once
// maui::core::swipe_view_handler  <=  Microsoft.Maui.Handlers.SwipeViewHandler
//
// The handler for a swipe view (ISwipeView) — a native container that hosts the single Content and runs
// the swipe state machine (Idle → Swiping → Open) over the four directional item sets. Ported from
// SwipeViewHandler.cs (+ SwipeViewHandler.iOS.cs + the native MauiSwipeView.cs state machine):
//   - Mapper: Content (MapContent → re-host the content's native view), SwipeTransitionMode, and the
//     four item collections (Left/Top/Right/BottomItems → no-op pushes on the platform's cached items,
//     matching C#'s empty MapLeftItems/... whose work is the gesture-time UpdateSwipeItems), chained onto
//     the shared view_mapper.
//   - CommandMapper: RequestOpen (ProgrammaticallyOpenSwipeItem) / RequestClose (ResetSwipe) — plus the
//     port's "set_content" runtime funnel every content host shares.
//
// THE STATE MACHINE lives in the platform struct (the MauiSwipeView twin). The headless backend ports
// the full machine — the open-threshold percentages (15% minimum-open, 60% open), the directional
// item-set selection, ValidateSwipeThreshold (Execute mode invokes the first VISIBLE item then
// closes/stays per SwipeBehaviorOnInvoked; Reveal mode swipes to the threshold), and the
// SwipeStarted/Changing/Ended notification fan-out back to the virtual view — driven by SYNTHETIC swipe
// offsets (begin_swipe / swipe_to / end_swipe) the tests inject, since headless has no real pan gesture.
// The Apple/iOS twins host the content on a real NSView/UIView with a pan recognizer (documented pan-
// driven reveal; the apple AppKit twin documents the no-native-swipe deviation — it reuses the same
// machine, driven by the programmatic open/close + the cross-platform offsets).
//
// Same partial-class split: tables + ctor cross-platform (swipe_view_handler.cpp); create + set_content +
// the state-machine drivers per backend under src/platform/<backend>/swipe_view_handler.{cpp,mm}.

#include <any>
#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/core/swipe_view_handler_state.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_view;

    // Derives view_platform_base so the shared view_mapper pushes the generic IView properties onto it.
    // Holds the swipe state machine (the MauiSwipeView twin) — the headless backend runs it fully;
    // the native backends host the content + a pan recognizer (see the header comment).
    struct swipe_view_platform : view_platform_base
    {
        swipe_view_platform() = default;
        ~swipe_view_platform() override; // backend-defined: releases the retained native container
        swipe_view_platform(const swipe_view_platform&) = delete;
        swipe_view_platform(swipe_view_platform&&) = delete;
        swipe_view_platform& operator=(const swipe_view_platform&) = delete;
        swipe_view_platform& operator=(swipe_view_platform&&) = delete;

        void* native = nullptr;
        // The hosted content child (the swipe content) — null when no content is set.
        i_view* hosted_content = nullptr;
        // Cached transition mode (MapSwipeTransitionMode).
        swipe_transition_mode transition = swipe_transition_mode::reveal;
        // The current state-machine snapshot the headless tests observe (state / direction / offset /
        // open). On the native backends this mirrors the real pan-driven reveal.
        swipe_view_state state;

#ifdef MAUI_PLATFORM_APPLE
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
        // The interactive drag-to-reveal pieces (W7-U09): the UIPanGestureRecognizer + its target
        // trampoline (retained void* so this struct stays Obj-C-free) drive the shared swipe_machine from
        // the real gesture (MauiSwipeView's _panGestureRecognizer + HandlePan). Released in the destructor.
        void* pan_recognizer = nullptr; // retained UIPanGestureRecognizer
        void* pan_target = nullptr;     // retained target trampoline (calls back into the handler)

        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: push the generic IView properties to the real dev.mauicpp.MauiLayout swipe
        // host (defined in src/platform/android/swipe_view_handler.cpp). The host hosts the single swipe
        // Content as its child — the STATIC render the gallery captures need (the live drag-to-reveal pan
        // is the documented deviation; see the .cpp header). is_enabled is intentionally NOT overridden — a
        // plain ViewGroup host has no enabled state, matching the apple/ios twins. Each override calls the
        // view_platform_base body FIRST (the VM-less cross-platform suite observes the headless mirror),
        // then pushes to the ViewGroup when one exists. Visibility/opacity/automation_id push directly;
        // transform/flow_direction/background/semantics push through the shared android ops. Shadow / Clip /
        // InputTransparent keep ONLY the base mirror (WrapperView-only on Android, no plain-View analog —
        // the same scope the content_page/layout partials document).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class swipe_view_handler : public view_handler<swipe_view_handler, i_swipe_view, swipe_view_platform>
    {
    public:
        swipe_view_handler();

        static property_mapper<i_swipe_view, swipe_view_handler>& mapper();
        static command_mapper<i_swipe_view, swipe_view_handler>& command_mapper();

        static std::unique_ptr<swipe_view_platform> create_platform_view();

#ifdef MAUI_PLATFORM_IOS
        // C# ConnectHandler (W7-U09 iOS): bind the pan-gesture target to this handler so the real
        // UIPanGestureRecognizer can drive the shared swipe machine. Detected by the base view_handler's
        // `if constexpr (requires …)` and called once on first connect; iOS-only (no native pan elsewhere).
        void on_connect_handler(swipe_view_platform& platform);
#endif

        // A swipe view computes its own size through the control (which ports MeasureContent), so the
        // handler reports nothing here.
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces (the state-machine drivers + content host) ----
        // Re-host the content's native view (C# MauiSwipeView.UpdateContent).
        void set_content();
        // Push the transition mode (C# MapSwipeTransitionMode).
        void update_transition_mode();
        // A collection changed (C# MapLeftItems/... — a no-op cache touch on the platform; the gesture-time
        // UpdateSwipeItems re-reads the live collection). Kept so a runtime collection change re-pushes.
        void update_items();
        // C# ProgrammaticallyOpenSwipeItem: open toward the requested side (drives the state to Open and
        // writes IsOpen back through the virtual view).
        void programmatically_open(const swipe_view_open_request& request);
        // C# ResetSwipe: close the view (state → Idle, IsOpen false).
        void reset_swipe(bool animated);

        // ---- the SYNTHETIC swipe pipeline (headless: the test-injected pan; the native pan recognizer
        // calls the same entry points). begin_swipe sets the direction + selects the item set; swipe_to
        // pushes a new offset (raising SwipeStarted once, then SwipeChanging, updating IsOpen); end_swipe
        // finishes (raising SwipeEnded, then ValidateSwipeThreshold: invoke + close, or swipe-to-threshold,
        // or reset). ----
        void begin_swipe(swipe_direction direction);
        void swipe_to(double offset);
        void end_swipe();

        // ---- mapper entries (cross-platform, funnel to the per-backend pieces) ----
        static void map_content(swipe_view_handler& handler, i_swipe_view& view);
        static void map_transition_mode(swipe_view_handler& handler, i_swipe_view& view);
        static void map_items(swipe_view_handler& handler, i_swipe_view& view);
        static void map_request_open(swipe_view_handler& handler, i_swipe_view& view, const std::any& args);
        static void map_request_close(swipe_view_handler& handler, i_swipe_view& view, const std::any& args);
        static void map_set_content(swipe_view_handler& handler, i_swipe_view& view, const std::any& args);
    };
} // namespace maui::core
