#pragma once
// maui::core::navigation_page_handler  <=  Microsoft.Maui.Handlers.NavigationViewHandler
//
// The handler behind a navigation_page: it owns a native CONTAINER view and hosts the navigation stack's
// current (top-most) page's native view inside it, swapping that subview on each push/pop. Ported from
// NavigationViewHandler.cs (+ the iOS UINavigationController host, simplified): on iOS the host is a
// UINavigationController that pushes/pops UIViewControllers; AppKit (macOS) has NO UINavigationController,
// so the host is a plain NSView CONTAINER that swaps the current page's native view (remove the old
// subview, add the new, frame to bounds), with NO animation. The transition is synchronous — the handler
// calls IStackNavigation.NavigationFinished inline once the swap is done (C#'s async completion signal).
//
// The drive is a COMMAND ("request_navigation", payload = a navigation_request) rather than a property
// map, mirroring C#'s Handler.Invoke(nameof(IStackNavigation.RequestNavigation), request): the control
// builds the request from its current stack and invokes it; the handler reads the request's top-most page
// and re-hosts it. Same partial-class split + single cross-platform navigation_page_platform struct as
// the other handlers: the mapper TABLES + ctor are cross-platform (navigation_page_handler.cpp); the
// platform recipe (create the container + the host_current subview swap) lives per backend under
// src/platform/<backend>/navigation_page_handler.{cpp,mm}.

#include <any>
#include <memory>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; Apple overrides update_* to push to the NSView container).
    struct navigation_page_platform : view_platform_base
    {
        navigation_page_platform() = default;
        ~navigation_page_platform() override; // backend-defined: releases the retained native container on Apple
        navigation_page_platform(const navigation_page_platform&) = delete;
        navigation_page_platform(navigation_page_platform&&) = delete;
        navigation_page_platform& operator=(const navigation_page_platform&) = delete;
        navigation_page_platform& operator=(navigation_page_platform&&) = delete;

        void* native = nullptr;
        // The currently-hosted (top-most) page — the container's mirror of the navigation stack's current
        // page (the Apple build ALSO re-parents the matching real NSView subview). Null when the stack is
        // empty; the headless tests observe this to confirm the host tracks the current page.
        i_view* hosted_page = nullptr;

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
    };

    class navigation_page_handler : public view_handler<navigation_page_handler, i_view, navigation_page_platform>
    {
    public:
        navigation_page_handler();

        static property_mapper<i_view, navigation_page_handler>& mapper();
        static command_mapper<i_view, navigation_page_handler>& command_mapper();

        static std::unique_ptr<navigation_page_platform> create_platform_view();

        // The navigation page computes its size from its current page, not the handler; the handler
        // reports nothing here (like layout_handler / content_page_handler).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        // Frame the container AND the current page to the bounds (the page fills the container).
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Host (or re-host) the new top-most page from the request's stack as the container's subview,
        // then report completion back to the view via i_stack_navigation::navigation_finished (defined
        // per backend). top is the request's last page (top-most), or null for an empty stack.
        void host_current(i_view* top);

        // The "request_navigation" COMMAND (C# MapRequestNavigation / Handler.Invoke(RequestNavigation)):
        // read the request's top-most page, re-host it, and report completion. The payload is a
        // navigation_request.
        static void map_request_navigation(navigation_page_handler& handler, i_view& view, const std::any& args);
    };
} // namespace maui::core
