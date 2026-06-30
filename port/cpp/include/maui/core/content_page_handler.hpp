#pragma once
// maui::core::content_page_handler  <=  Microsoft.Maui.Handlers.ContentViewHandler
//
// The handler for a content view/page (IContentView) — the native host behind a content_page control.
// Like a layout, a content view computes its OWN geometry (the control's measure/arrange port C#'s
// MeasureContent/ArrangeContent); the handler's job is to HOST the single content child: it owns a
// native container view and re-parents the content's native view as that container's subview whenever
// the content changes, via the command seam ("set_content"). Ported from ContentViewHandler.cs +
// ContentViewHandler.iOS.cs (the AppKit host is a plain NSView container — no NSViewController, kept
// minimal).
//
// Same partial-class split + single cross-platform content_page_platform struct as the other handlers:
// the mapper TABLES + ctor are cross-platform (content_page_handler.cpp); the platform recipe (create +
// the set_content subview re-host) lives per backend under
// src/platform/<backend>/content_page_handler.{cpp,mm}.
//
// C#'s MapContent is a PROPERTY map ("Content" → MapContent → UpdateContent), so it runs on connect
// (the initial UpdateProperties) AND on every Content change. We keep that: a "content" PROPERTY entry
// re-hosts on connect (so content set BEFORE the handler is attached is hosted), mirroring C#. The
// control ALSO routes runtime content changes through a "set_content" COMMAND (mirroring layout_handler's
// add/remove child seam, and the M4c task's command requirement); both triggers funnel to the single
// set_content() re-host, which reads the new content from the virtual view (payload-free).

#include <any>
#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/platform/ios/hide_soft_input_on_tapped_manager.hpp" // the per-handler HideSoftInputOnTapped manager

namespace maui::core
{
    class i_view;

    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; Apple overrides update_* to push to the NSView host).
    struct content_page_platform : view_platform_base
    {
        content_page_platform() = default;
        ~content_page_platform() override; // backend-defined: releases the retained native host on Apple
        content_page_platform(const content_page_platform&) = delete;
        content_page_platform(content_page_platform&&) = delete;
        content_page_platform& operator=(const content_page_platform&) = delete;
        content_page_platform& operator=(content_page_platform&&) = delete;

        void* native = nullptr;
        // The hosted content child — the host's mirror of the control's content (the Apple build ALSO
        // re-parents the matching real NSView subview). Null when no content is set; the headless tests
        // observe this to confirm the host tracks the control's content.
        i_view* hosted_content = nullptr;

        // --- platform configuration (W2-24): the iOSSpecific Page nudge mirrors. C#'s PageHandler maps
        // the IiOSPageSpecifics keys to ViewController.SetNeedsStatusBarAppearanceUpdate() /
        // SetNeedsUpdateOfHomeIndicatorAutoHidden(); every backend counts the request (the seam tests
        // observe it), the iOS twin additionally pokes the host's owning UIViewController. ---
        int status_bar_appearance_requests = 0;
        int home_indicator_requests = 0;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSView host (defined in
        // src/platform/apple/content_page_handler.mm). is_enabled is intentionally NOT overridden — a
        // plain NSView host has no enabled state (unlike NSControl), so it keeps the base mirror.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the page's NSView (M5d native
        // a11y / hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent →
        // -hitTest: gate.
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the UIView host (defined in
        // src/platform/ios/content_page_handler.mm). is_enabled is intentionally NOT overridden — a
        // plain UIView host has no enabled state (only UIControl has), so it keeps the base mirror.
        // transform IS pushed via the shared ios apply_transform helper (the generic-IView ViewMapper
        // widening); flow_direction still keeps the base mirror for now (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background / shadow / clip pushed to the host's layer (ios_visual_ops.hpp) + semantics /
        // input-transparent (ios_semantics_ops.hpp: accessibilityLabel/Hint + the Header trait,
        // userInteractionEnabled) — the direct iOS C# extension ports.
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: push the generic IView properties to the real dev.mauicpp.MauiLayout content
        // host (defined in src/platform/android/content_page_handler.cpp). is_enabled is intentionally NOT
        // overridden — a plain ViewGroup host has no enabled state, matching the apple/ios twins. Each
        // override calls the view_platform_base body FIRST (the VM-less cross-platform suite observes the
        // headless mirror), then pushes to the ViewGroup when one exists. Visibility/opacity/automation_id
        // push directly; transform/flow_direction/background/semantics push through the shared android
        // ops. Shadow / Clip / InputTransparent keep ONLY the base mirror (WrapperView-only on Android,
        // no plain-View analog — the same scope the button/layout partials document).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class content_page_handler : public view_handler<content_page_handler, i_content_view, content_page_platform>
    {
    public:
        content_page_handler();

        static property_mapper<i_content_view, content_page_handler>& mapper();
        static command_mapper<i_content_view, content_page_handler>& command_mapper();

        static std::unique_ptr<content_page_platform> create_platform_view();

        // A content view computes its own size through the control (which ports MeasureContent), so the
        // handler reports nothing here — like layout_handler.
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Re-host the content subview from the virtual view's current content (defined per backend).
        void set_content();

        // The "content" PROPERTY map (C# MapContent): re-host the content. Runs on connect and on every
        // content change routed through the property path.
        static void map_content(content_page_handler& handler, i_content_view& view);

        // The "set_content" COMMAND: re-host the content (the runtime trigger the control invokes on a
        // content change). Reads the new content from the virtual view (no payload needed), mirroring C#'s
        // UpdateContent reading VirtualView.PresentedContent.
        static void map_set_content(content_page_handler& handler, i_content_view& view, const std::any& args);

        // --- platform configuration (W2-24): the iOSSpecific Page knob nudges (C# PageHandler.iOS
        // MapPrefersStatusBarHiddenMode / MapHomeIndicatorAutoHidden — the C# double indirection through
        // ContentPage.RemapForControls + UpdateValue(IiOSPageSpecifics key) collapses to mapping the
        // namespaced knob names the store raises). Defined per backend.
        static void map_prefers_status_bar_hidden(content_page_handler& handler, i_content_view& view);
        static void map_home_indicator_auto_hidden(content_page_handler& handler, i_content_view& view);

        // C# ContentPage.MapHideSoftInputOnTapped → page.UpdateHideSoftInputOnTapped() →
        // manager.UpdatePage(page). The port resolves the manager from this handler (see soft_input_manager
        // below) instead of DI, then routes the content_page through it. Runs on connect AND on every
        // HideSoftInputOnTapped change routed through the property path (key "hide_soft_input_on_tapped").
        static void map_hide_soft_input_on_tapped(content_page_handler& handler, i_content_view& view);

        // C# SafeAreaElement / ContentPage.SafeAreaEdges: a change to the per-edge safe-area knob re-runs
        // the layout (the native host insets per GetSafeAreaRegionsForEdge). The port invalidates the
        // page's measure so MeasureContent/ArrangeContent re-fold the new per-edge insets. Runs on connect
        // AND on every SafeAreaEdges change routed through the property path (key "safe_area_edges").
        static void map_safe_area_edges(content_page_handler& handler, i_content_view& view);

        // The per-handler HideSoftInputOnTapped manager (C#'s scoped HideSoftInputOnTappedChangedManager
        // service). InputView focus changes (VisualElement.Focused/Unfocused → C# InputView.MapIsFocused)
        // route through update_focus_for_view; the page mapper routes through update_page.
        [[nodiscard]] maui::platform::ios::hide_soft_input_on_tapped_manager& soft_input_manager()
        {
            return soft_input_manager_;
        }

        // C# OnConnectHandler/OnDisconnectHandler analog: the iOS twin wires the host UIView back to this
        // handler so safeAreaInsetsDidChange can push the realized insets through i_safe_area_view2
        // (MauiView.SafeAreaInsetsDidChange); headless/AppKit define both empty.
        void on_connect_handler(content_page_platform& platform);
        static void on_disconnect_handler(content_page_platform& platform);

    private:
        maui::platform::ios::hide_soft_input_on_tapped_manager soft_input_manager_;
    };
} // namespace maui::core
