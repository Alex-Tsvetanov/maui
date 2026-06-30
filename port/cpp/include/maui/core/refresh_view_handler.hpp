#pragma once
// maui::core::refresh_view_handler  <=  Microsoft.Maui.Handlers.RefreshViewHandler
//
// The handler for a refresh view (IRefreshView) — a native container that hosts the single Content and
// drives pull-to-refresh. Ported from RefreshViewHandler.cs + RefreshViewHandler.iOS.cs:
//   - Mapper: IsRefreshing (MapIsRefreshing → the native control's refreshing state), Content
//     (MapContent → re-host the content's native view), RefreshColor (MapRefreshColor → the spinner
//     tint), IsRefreshEnabled (MapIsRefreshEnabled → enable/disable the pull gesture), chained onto the
//     shared view_mapper. No CommandMapper beyond the port's "set_content" content funnel.
//   - The USER pull writes back through the virtual view: the native control's ValueChanged
//     (MauiRefreshViewProxy.OnRefresh) sets IRefreshView.IsRefreshing = true, which re-enters the
//     control's coercion (raises Refreshing + runs the command). The handler exposes
//     request_refresh() — the headless / programmatic stand-in for that native pull — which the tests
//     call to simulate the gesture.
//
// PLATFORM notes (documented):
//   - iOS: a real UIRefreshControl on the hosted scroll content (the MauiRefreshView recipe).
//   - macOS (AppKit): AppKit has NO native pull-to-refresh control, so the apple twin is a documented
//     deviation — IsRefreshing drives a stored spinner-overlay flag (no native spinner widget); the
//     content is hosted on a plain NSView; request_refresh() still flips IsRefreshing for parity.
//   - headless: mirrors IsRefreshing / IsRefreshEnabled / the spinner color / the hosted content, and
//     request_refresh() writes IsRefreshing=true back through the virtual view (the ValueChanged twin).
//
// Same partial-class split: tables + ctor cross-platform (refresh_view_handler.cpp); create +
// set_content + the update_* pushes + request_refresh per backend under
// src/platform/<backend>/refresh_view_handler.{cpp,mm}.

#include <any>
#include <cstdint>
#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_view;

    // Derives view_platform_base so the shared view_mapper pushes the generic IView properties onto it.
    struct refresh_view_platform : view_platform_base
    {
        refresh_view_platform() = default;
        ~refresh_view_platform() override; // backend-defined: releases the retained native container
        refresh_view_platform(const refresh_view_platform&) = delete;
        refresh_view_platform(refresh_view_platform&&) = delete;
        refresh_view_platform& operator=(const refresh_view_platform&) = delete;
        refresh_view_platform& operator=(refresh_view_platform&&) = delete;

        void* native = nullptr;
        // The hosted scrollable content child — null when no content is set.
        i_view* hosted_content = nullptr;
        // Cross-backend mirrors of the mapped refresh surface (headless asserts on them; the native
        // partials keep them current beside the real pushes).
        bool refreshing = false;
        bool refresh_enabled = true;
        // Whether a spinner color was pushed + its packed ARGB (the headless / AppKit spinner mirror).
        bool has_refresh_color = false;
        std::uint32_t refresh_color_argb = 0;

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
        // Android backend: push the generic IView properties to the real dev.mauicpp.MauiLayout refresh
        // host (defined in src/platform/android/refresh_view_handler.cpp). The host hosts the single
        // scrollable Content as its child — the STATIC render the gallery captures need (the live
        // pull-to-refresh gesture + the spinner are the documented deviation: MAUI's
        // MauiSwipeRefreshLayout : AndroidX.SwipeRefreshLayout is unavailable on this AAR-less backend,
        // so the host is a plain MauiLayout with no pull recognizer / spinner widget; see the .cpp
        // header). is_enabled is intentionally NOT overridden — a plain ViewGroup host has no enabled
        // state, matching the apple/ios twins. Each override calls the view_platform_base body FIRST (the
        // VM-less cross-platform suite observes the headless mirror), then pushes to the ViewGroup when one
        // exists. Visibility/opacity/automation_id push directly;
        // transform/flow_direction/background/semantics push through the shared android ops. Shadow / Clip
        // / InputTransparent keep ONLY the base mirror (WrapperView-only on Android, no plain-View analog —
        // the same scope the content_page/swipe_view partials document).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class refresh_view_handler : public view_handler<refresh_view_handler, i_refresh_view, refresh_view_platform>
    {
    public:
        refresh_view_handler();

        static property_mapper<i_refresh_view, refresh_view_handler>& mapper();
        static command_mapper<i_refresh_view, refresh_view_handler>& command_mapper();

        static std::unique_ptr<refresh_view_platform> create_platform_view();

        // A refresh view computes its own size through the control (which ports MeasureContent).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces ----
        void set_content();
        void update_is_refreshing();
        void update_refresh_color();
        void update_is_refresh_enabled();
        // The native pull stand-in (MauiRefreshViewProxy.OnRefresh): write IsRefreshing=true back through
        // the virtual view (which re-enters the control's coercion → Refreshing + command). The tests call
        // this to simulate the user pulling to refresh.
        void request_refresh();

        // ---- mapper entries ----
        static void map_is_refreshing(refresh_view_handler& handler, i_refresh_view& view);
        static void map_content(refresh_view_handler& handler, i_refresh_view& view);
        static void map_refresh_color(refresh_view_handler& handler, i_refresh_view& view);
        static void map_is_refresh_enabled(refresh_view_handler& handler, i_refresh_view& view);
        static void map_set_content(refresh_view_handler& handler, i_refresh_view& view, const std::any& args);
    };
} // namespace maui::core
