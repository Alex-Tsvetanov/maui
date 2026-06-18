#pragma once
// gallery_attach — the shared attach + re-host helpers every demo page's attach_handlers(maui_app) uses.
//
// Two problems this solves, both stemming from the demo pages building their element tree in the
// CONSTRUCTOR (before any handler exists), then the gallery main attaching handlers bottom-up afterward:
//
//   1. attach: attach_handler<View> keys on the CONCRETE static type (type_tag::of<View>()), and throws
//      when no handler is registered for that type (e.g. several value controls have no AppKit handler).
//      gallery_attach_one wraps each attach in a try/catch that logs + continues, so one unregistered
//      control can't abort the page. The generic `auto&` parameter preserves each member's concrete type.
//
//   2. re-host: the native panels mirror their children only via the host COMMANDS ("add" for layouts,
//      "set_content" for content hosts, "items_source"/"flyout"+"detail" for the multi-page hosts) — and
//      those commands fired during construction found NO handler attached, so nothing was hosted. Once
//      the handlers are attached, the page replays them through these gallery_rehost_* helpers so the
//      native subview tree actually materializes. Without this the window renders blank. (This is a
//      gallery-host concern, not a framework change: the framework wires the tree correctly when handlers
//      are attached BEFORE the children are added, as the controls' own tests do.)

#include <cstdio>
#include <exception>

#include "maui/controls/content_page.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    // Attach a handler to one OWNED view, guarded: an unregistered control logs + is skipped (the page
    // keeps mounting the rest). The generic `auto&` preserves the concrete static type attach_handler
    // keys on.
    template <class View> void gallery_attach_one(maui::hosting::maui_app& app, View& view, const char* name)
    {
        try
        {
            app.attach_handler(view);
        }
        catch (const std::exception& error)
        {
            std::fprintf(stderr, "[gallery] skip %s: %s\n", name, error.what());
        }
    }

    // Replay a layout's existing children into its (now-attached) handler so the native panel hosts each
    // child whose handler resolved (the "add" command — children whose attach threw hand back no native
    // view and are silently ignored by the panel).
    template <class Layout> void gallery_rehost_layout(Layout& layout)
    {
        if (const auto& layout_handler = layout.handler())
        {
            for (int i = 0; i < layout.count(); ++i)
            {
                layout_handler->invoke("add", maui::core::layout_handler_update{.index = i, .view = &layout.at(i)});
            }
        }
    }

    // Re-fire a content host's "set_content" so its (now-attached) handler hosts the content view. Works
    // for every single-content host whose set_content routes through the "set_content" command:
    // content_page, content_view, border, frame, scroll_view, refresh_view, swipe_view.
    template <class Host> void gallery_rehost_content(Host& host)
    {
        if (const auto& host_handler = host.handler())
        {
            host_handler->invoke("set_content");
        }
    }

    // Re-fire a tabbed_page's "items_source" so its (now-attached) handler hosts the tab pages (the
    // multi_page child-sync command).
    inline void gallery_rehost_pages(maui::controls::tabbed_page& tabs)
    {
        if (const auto& tabs_handler = tabs.handler())
        {
            tabs_handler->update_value("items_source");
        }
    }

    // Re-fire a flyout_page's flyout + detail pane maps so its (now-attached) handler hosts both panes.
    inline void gallery_rehost_panes(maui::controls::flyout_page& flyout)
    {
        if (const auto& flyout_handler = flyout.handler())
        {
            flyout_handler->update_value("flyout");
            flyout_handler->update_value("detail");
        }
    }
} // namespace maui::samples
