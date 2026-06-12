#pragma once
// maui::core::scroll_view_handler  <=  Microsoft.Maui.Handlers.ScrollViewHandler
//
// The handler for a scroll view (IScrollView) — a native scroller hosting the single content child's
// native view as its scrollable document. Ported from ScrollViewHandler.cs + ScrollViewHandler.iOS.cs:
//   - Mapper: Content (MapContent → UpdateContentView: clear + re-parent the content's native view),
//     Orientation (MapOrientation → re-derive the native scroll-ability), and the two scroll-bar
//     visibilities; chained onto the shared view_mapper.
//   - CommandMapper: RequestScrollTo (MapRequestScrollTo: clamp the target to the available scroll
//     range, move the native offset, and acknowledge ScrollFinished) — plus the port's "set_content"
//     runtime funnel shared by every content host.
//   - The native partial pushes USER scrolls back through i_scroll_view::set_horizontal_offset/
//     set_vertical_offset (the ScrollEventProxy.Scrolled write-back) and reports animation completion
//     through scroll_finished() (ScrollAnimationEnded).
//
// The control computes its own geometry (the ScrollView CrossPlatformMeasure/ArrangeContentUnbounded
// pair lives on the control, like every port content host); platform_arrange frames the native
// scroller AND re-derives the scrollable extent from the freshly-arranged content (the MauiScrollView
// LayoutSubviews → ContentSize push). Headless mirrors orientation / bar visibilities / content /
// offsets and RECORDS every scroll_to request (then completes it synchronously: offsets write back and
// scroll_finished fires — the platform loop collapsed, as navigation does).
//
// Same partial-class split: tables + ctor cross-platform (scroll_view_handler.cpp); create +
// set_content + update_* + scroll_to + arrange per backend under
// src/platform/<backend>/scroll_view_handler.{cpp,mm}.

#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_scroll_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_view;

    // Derives view_platform_base so the shared view_mapper pushes the generic IView properties onto it.
    struct scroll_view_platform : view_platform_base
    {
        scroll_view_platform() = default;
        ~scroll_view_platform() override; // backend-defined: releases the retained native scroller
        scroll_view_platform(const scroll_view_platform&) = delete;
        scroll_view_platform(scroll_view_platform&&) = delete;
        scroll_view_platform& operator=(const scroll_view_platform&) = delete;
        scroll_view_platform& operator=(scroll_view_platform&&) = delete;

        void* native = nullptr;
        // The hosted content child (the scroller's document) — null when no content is set.
        i_view* hosted_content = nullptr;
        // Cross-backend mirrors of the mapped scroll surface (headless asserts on them; the native
        // partials keep them current beside the real pushes).
        scroll_orientation orientation = scroll_orientation::vertical;
        scroll_bar_visibility horizontal_bar_visibility = scroll_bar_visibility::default_;
        scroll_bar_visibility vertical_bar_visibility = scroll_bar_visibility::default_;
        double offset_x = 0;
        double offset_y = 0;
        // Every executed RequestScrollTo, in order (the headless record the task spec asks for; the
        // native partials append too, so on-device tests can assert the same trail).
        std::vector<scroll_to_request> scroll_requests;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend (src/platform/apple/scroll_view_handler.mm): the generic IView pushes onto the
        // NSScrollView. is_enabled keeps the base mirror (scroll-ability is derived from orientation).
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
        // iOS backend (src/platform/ios/scroll_view_handler.mm): the UIScrollView twin. is_enabled and
        // transform/flow_direction keep the base mirrors (the content_page partial's scope).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif
    };

    class scroll_view_handler : public view_handler<scroll_view_handler, i_scroll_view, scroll_view_platform>
    {
    public:
        scroll_view_handler();

        static property_mapper<i_scroll_view, scroll_view_handler>& mapper();
        static command_mapper<i_scroll_view, scroll_view_handler>& command_mapper();

        static std::unique_ptr<scroll_view_platform> create_platform_view();

        // The scroll view computes its own size through the control (the handler-side
        // CrossPlatformMeasure lives there), so the handler reports nothing here.
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        // Frame the native scroller and push the scrollable extent (the MauiScrollView LayoutSubviews →
        // ContentSize update; defined per backend).
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- per-backend pieces ----
        // C# ConnectHandler/DisconnectHandler: wire (and tear down) the native scrolled write-back —
        // the ScrollEventProxy.Connect/Disconnect pair. No-ops on headless.
        void on_connect_handler(scroll_view_platform& platform);
        static void on_disconnect_handler(scroll_view_platform& platform);
        // Re-host the content's native view as the scrollable document (C# UpdateContentView).
        void set_content();
        // Push orientation (C# MapOrientation: re-derives the native scroll-ability + invalidates).
        void update_orientation();
        // Push the scroll-bar visibilities (C# Map*ScrollBarVisibility).
        void update_horizontal_scroll_bar_visibility();
        void update_vertical_scroll_bar_visibility();
        // Execute a scroll-to (C# MapRequestScrollTo: clamp, move, acknowledge ScrollFinished).
        void scroll_to(const scroll_to_request& request);

        // ---- mapper entries (cross-platform, funnel to the per-backend pieces) ----
        static void map_content(scroll_view_handler& handler, i_scroll_view& view);
        static void map_orientation(scroll_view_handler& handler, i_scroll_view& view);
        static void map_horizontal_scroll_bar_visibility(scroll_view_handler& handler, i_scroll_view& view);
        static void map_vertical_scroll_bar_visibility(scroll_view_handler& handler, i_scroll_view& view);
        static void map_request_scroll_to(scroll_view_handler& handler, i_scroll_view& view, const std::any& args);
        static void map_set_content(scroll_view_handler& handler, i_scroll_view& view, const std::any& args);
    };
} // namespace maui::core
