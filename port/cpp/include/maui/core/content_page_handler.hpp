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
    };
} // namespace maui::core
