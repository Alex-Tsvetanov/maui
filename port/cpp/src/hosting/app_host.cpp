// maui::hosting::app_host — the generic recursive mount + layout driver (app_host.hpp).
//
// Extracts the per-page mount the gallery mains hand-rolled (attach_handlers + gallery_rehost_* +
// boot_page's measure/arrange) into a backend-agnostic driver that walks an arbitrary element tree via the
// element generic-mount surface (visit_logical_children / handler_type_tag / mount_into_handler). No
// per-control special-casing: every container already exposes its children + host command through those
// three element hooks.

#include "maui/hosting/app_host.hpp"

#include <cstdio>
#include <memory>
#include <optional>

#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::hosting
{
    namespace
    {
        // Attach the registered handler to ONE element by its runtime handler_type_tag (the non-template
        // analog of maui_app::attach_handler<View>, which keys on the static type). Mirrors that path:
        // resolve the factory, set the maui context BEFORE the virtual view (as C# SetMauiContext precedes
        // SetVirtualView), then let the view own the handler. Returns false (skips) when the element declares
        // no handler tag (a leaf chrome element) or no handler is registered for it — a log + continue, so
        // one unmappable control can't abort the mount (matching gallery_attach_one's guard).
        bool attach_one(maui_app& app, maui::controls::element& view)
        {
            const std::optional<maui::core::type_tag> tag = view.handler_type_tag();
            if (!tag.has_value())
            {
                return false; // no handler to attach (intentional — e.g. a non-view chrome element)
            }
            // element is a sibling base of i_element (both are bases of the concrete view<>/window), so the
            // i_element seam (set_handler) is reached by a runtime cross-cast through the most-derived type.
            // A declared tag without an i_element face would be a control bug — guard rather than risk UB.
            auto* element_face = dynamic_cast<maui::core::i_element*>(&view);
            if (element_face == nullptr)
            {
                return false;
            }
            std::shared_ptr<maui::core::i_element_handler> handler = app.handlers().create_handler(*tag);
            if (handler == nullptr)
            {
                std::fprintf(stderr, "[app_host] skip element: no handler registered for its type\n");
                return false;
            }
            handler->set_maui_context(&app.context());     // SetMauiContext precedes SetVirtualView (C# order)
            element_face->set_handler(std::move(handler)); // the view owns its handler (PROFILE §11)
            return true;
        }
    } // namespace

    void mount_tree(maui_app& app, maui::controls::element& root)
    {
        // Depth-first POST-ORDER: attach every child's subtree FIRST, so each child's native view exists
        // before its parent hosts it (the window/native panels host by reading a child handler's native
        // view). visit_logical_children is the public window onto for_each_logical_child every container
        // overrides — the only traversal primitive the driver needs.
        root.visit_logical_children([&app](maui::controls::element& child) { mount_tree(app, child); });

        // Then THIS element: attach its own handler, then re-host its (now-attached) children's native views
        // into it via its own host command (mount_into_handler is a no-op on a leaf). Order matters: the
        // handler must exist before mount_into_handler fires its "set_content" / "add" command.
        attach_one(app, root);
        root.mount_into_handler();
    }

    void mount_window(maui_app& app, maui::controls::window& window)
    {
        // (1) Mount the page tree FIRST (handlers bottom-up + container re-host) so the page's view-handler
        //     exists — the window handler hosts the page by reading that handler's native view on open.
        if (auto* page = window.content_element())
        {
            mount_tree(app, *page);
        }
        // (2) Attach the window's own handler (the window is an element, not a view<> — window_handler), then
        // (3) open it through the application lifecycle: open_window drives Created/Activated AND the window
        //     handler's content hosting (MapContent), which now finds the page handler attached.
        app.attach_handler(window);
        app.open_window(window);
    }

    maui::graphics::size drive_layout(maui::controls::window& window, double width, double height)
    {
        // The window host does no auto-layout (mirrors the native backends — the run loop / gallery main
        // drives the pass). Measure then arrange the content page over the content bounds; a native backend
        // insets by the safe area (Stage 2) — headless uses the full {0,0,width,height}.
        auto* page = dynamic_cast<maui::core::i_view*>(window.content());
        if (page == nullptr)
        {
            return {0, 0};
        }
        page->measure(width, height);
        return page->arrange(maui::graphics::rect{0, 0, width, height});
    }
} // namespace maui::hosting
