#pragma once
// maui::core::navigation_page_handler  <=  Microsoft.Maui.Handlers.NavigationViewHandler
//
// The handler behind a navigation_page: it owns a native CONTAINER view that stacks a custom navigation
// BAR above a CONTENT area, and hosts the navigation stack's current (top-most) page's native view inside
// that content area, swapping that subview on each push/pop. Ported from NavigationViewHandler.cs (+ the
// iOS UINavigationController host, simplified): on iOS the host is a UINavigationController that pushes/
// pops UIViewControllers and supplies the navigation bar; AppKit (macOS) has NO UINavigationController, so
// the host is a plain NSView CONTAINER holding a CUSTOM bar (an NSView with an NSTextField title + a back
// NSButton, built here) above the content area; host_current swaps the content subview (remove the old,
// add the new, frame to bounds) and update_bar populates the bar from the view's chrome state
// (navigation_bar_title / navigation_back_button_visible). The content swap CROSS-FADES when the request
// is animated (CoreAnimation), and is instant otherwise; either way the transition is synchronous — the
// handler calls IStackNavigation.NavigationFinished inline once the swap is done (C#'s async completion).
// The back button's action routes to navigation_page::send_back_button_pressed() (→ pop()).
//
// A SECOND command, "request_modal_navigation", drives the MODAL overlay: the handler overlays the top
// modal's native view on top of the whole container (a modal NSView overlay sized to the container — the
// simpler-faithful AppKit choice vs. a child NSWindow, chosen so the modal participates in the same view
// tree and the headless mirror is observable), removing it when the modal stack empties.
//
// Both drives are COMMANDS (payload = a navigation_request), mirroring C#'s Handler.Invoke(
// nameof(IStackNavigation.RequestNavigation), request): the control builds the request from its current
// (modal) stack and invokes it; the handler reads the request's top-most page and re-hosts/overlays it.
// Same partial-class split + single cross-platform navigation_page_platform struct as the other handlers:
// the mapper TABLES + ctor are cross-platform (navigation_page_handler.cpp); the platform recipe (create
// the container + bar, the host_current content swap, the modal overlay) lives per backend under
// src/platform/<backend>/navigation_page_handler.{cpp,mm}.

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector> // --- chrome (W1-11): the toolbar_items mirror ---

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_toolbar_item; // --- chrome (W1-11): the toolbar_items mirror below borrows these ---

    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; Apple overrides update_* to push to the NSView container).
    struct navigation_page_platform : view_platform_base
    {
        navigation_page_platform() = default;
        ~navigation_page_platform() override; // backend-defined: releases the retained native views on Apple
        navigation_page_platform(const navigation_page_platform&) = delete;
        navigation_page_platform(navigation_page_platform&&) = delete;
        navigation_page_platform& operator=(const navigation_page_platform&) = delete;
        navigation_page_platform& operator=(navigation_page_platform&&) = delete;

        void* native = nullptr; // the container NSView (bar + content area) on Apple
        // The currently-hosted (top-most) page — the container's mirror of the navigation stack's current
        // page (the Apple build ALSO re-parents the matching real NSView subview). Null when the stack is
        // empty; the headless tests observe this to confirm the host tracks the current page.
        i_view* hosted_page = nullptr;
        // The currently-overlaid modal (top of the modal stack), or null when no modal is presented. The
        // Apple build overlays the matching real NSView; headless mirrors only the pointer.
        i_view* hosted_modal = nullptr;
        // The navigation chrome mirrors (headless-observable; the Apple bar's NSTextField/NSButton are
        // populated from these). bar_title = the current page's Title; back_button_visible = depth > 1.
        std::string bar_title;
        bool back_button_visible = false;
        // Bar styling mirrors (C# BarBackgroundColor / BarTextColor / TitleView). The colors are nullopt when
        // the developer never set them (the Apple bar then keeps its system default); when set, the Apple bar
        // is painted with them. hosted_title_view = the view shown in the bar instead of the title label
        // (null = the title label). All headless-observable so the seam is testable without a real bar.
        std::optional<maui::graphics::color> bar_background_color;
        std::optional<maui::graphics::color> bar_text_color;
        i_view* hosted_title_view = nullptr;
        // The last navigation request's animated flag (headless-observable; the Apple twin cross-fades the
        // content swap when true). Mirrors NavigationRequest.Animated for the realized transition.
        bool last_animated = false;
        // --- platform configuration (W2-24): the realized iOSSpecific IsNavigationBarTranslucent knob
        // (every backend keeps the mirror; the iOS twin additionally gives the custom bar a blur backdrop
        // and lets the content extend under it — the UINavigationBar.Translucent analog). ---
        bool bar_translucent = false;

        // --- chrome (W1-11): the page-surfaced toolbar items ---------------------------------------------
        // Mirror of i_stack_navigation::navigation_toolbar_items() (the ToolbarTracker aggregate),
        // refreshed by host_current. The iOS twin ADDITIONALLY materializes them as buttons on the right
        // of the custom navigation bar (C#'s UINavigationBar rightBarButtonItems path; secondary items
        // follow the primaries — documented simplification of the overflow). AppKit keeps the mirror only
        // — its toolbar items surface through the window's NSToolbar (window_handler) instead.
        std::vector<i_toolbar_item*> toolbar_items;
#ifdef MAUI_PLATFORM_IOS
        void* toolbar_buttons = nullptr; // retained NSMutableArray<UIButton*> (the bar's right buttons)
        void* toolbar_targets = nullptr; // retained NSMutableArray of click trampolines (button → send_clicked)
        void* bar_backdrop = nullptr;    // retained UIVisualEffectView behind the bar while translucent (W2-24)
#endif
        // --- end chrome (W1-11) --------------------------------------------------------------------------

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        // The retained native chrome slots, shared by the two real-native twins (NSView/NSTextField/
        // NSButton on macOS; UIView/UILabel/UIButton on iOS — same custom-bar recipe on both).
        void* bar = nullptr;             // the custom bar view (subview of the container)
        void* title_field = nullptr;     // the bar's title (NSTextField / UILabel)
        void* back_button = nullptr;     // the bar's back button (NSButton / UIButton)
        void* back_target = nullptr;     // the back button's retained target-action trampoline
        void* modal_overlay = nullptr;   // the presented modal's wrapper view overlaying the container
        void* title_view_host = nullptr; // the hosted TitleView's native view (retained while in the bar)
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSView container (defined in
        // src/platform/apple/navigation_page_handler.mm). is_enabled is intentionally NOT overridden — a
        // plain NSView container has no enabled state (unlike NSControl), so it keeps the base mirror.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the UIView container (defined in
        // src/platform/ios/navigation_page_handler.mm). is_enabled is intentionally NOT overridden — a
        // plain UIView container has no enabled state (only UIControl has), so it keeps the base mirror.
        // transform IS pushed via the shared ios apply_transform helper (the generic-IView ViewMapper
        // widening); flow_direction still keeps the base mirror for now (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background / shadow / clip pushed to the container's layer (ios_visual_ops.hpp) + semantics /
        // input-transparent (ios_semantics_ops.hpp) — the direct iOS C# extension ports.
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif
    };

    class navigation_page_handler : public view_handler<navigation_page_handler, i_view, navigation_page_platform>
    {
    public:
        navigation_page_handler();

        static property_mapper<i_view, navigation_page_handler>& mapper();
        static command_mapper<i_view, navigation_page_handler>& command_mapper();

        static std::unique_ptr<navigation_page_platform> create_platform_view();

        // C# OnConnectHandler: wire the bar's back-button target-action trampoline to this handler (Apple
        // only — the trampoline routes the click to i_stack_navigation::send_back_button_pressed). Detected
        // by the view_handler base via `if constexpr (requires …)`; the headless backend defines it empty.
        void on_connect_handler(navigation_page_platform& platform);

        // The navigation page computes its size from its current page, not the handler; the handler
        // reports nothing here (like layout_handler / content_page_handler).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        // Frame the container, its bar + content area, and the current page (the page fills the content).
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Host (or re-host) the new top-most page from the request's stack as the content subview (with a
        // cross-fade when `animated`), update the bar from `view`'s chrome state, then report completion
        // back to the view via i_stack_navigation::navigation_finished (defined per backend). `top` is the
        // request's last page (top-most), or null for an empty stack.
        void host_current(i_view* top, i_view& view, bool animated);

        // Overlay (or clear) the top modal's native view on top of the whole container. `top_modal` is the
        // modal request's last page (top-most modal), or null to clear the overlay.
        void host_modal(i_view* top_modal, bool animated);

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        // Host (or clear) the bar's TitleView (C# NavigationPage.TitleView): when set, hide the title label
        // and add the title view's native view to the bar; when null, remove the previously-hosted title
        // view and show the label again. Real-native twins only (the headless platform mirrors the pointer
        // in host_current). Static-free member so it can be called from host_current; takes the platform
        // slot.
        static void host_title_view(navigation_page_platform& platform, i_view* title_view);
#endif

        // The "request_navigation" COMMAND (C# MapRequestNavigation / Handler.Invoke(RequestNavigation)):
        // read the request's top-most page, re-host it, update the bar, and report completion. Payload =
        // a navigation_request.
        static void map_request_navigation(navigation_page_handler& handler, i_view& view, const std::any& args);

        // The "request_modal_navigation" COMMAND: read the modal request's top-most modal and overlay it
        // (or clear the overlay when the modal stack is empty). Payload = a navigation_request.
        static void map_request_modal_navigation(navigation_page_handler& handler, i_view& view, const std::any& args);

        // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent map. The
        // cross-platform map (navigation_page_handler.cpp) reads the i_stack_navigation face and routes
        // to the per-backend push below (headless/AppKit: the mirror; iOS: blur backdrop + content frame).
        static void map_is_navigation_bar_translucent(navigation_page_handler& handler, i_view& view);
        void update_bar_translucent(bool value);
    };
} // namespace maui::core
