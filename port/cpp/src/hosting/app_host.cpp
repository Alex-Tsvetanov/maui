// maui::hosting::app_host — the generic recursive mount + layout driver (app_host.hpp).
//
// Extracts the per-page mount the gallery mains hand-rolled (attach_handlers + gallery_rehost_* +
// boot_page's measure/arrange) into a backend-agnostic driver that walks an arbitrary element tree via the
// element generic-mount surface (visit_logical_children / handler_type_tag / mount_into_handler). No
// per-control special-casing: every container already exposes its children + host command through those
// three element hooks.

#if defined(__APPLE__)
    #include <TargetConditionals.h> // TARGET_OS_MACCATALYST — the safe-area margin fork below is iOS-only
#endif

#include "maui/hosting/app_host.hpp"
#include <algorithm>

#include <cstdio>
#include <memory>
#include <optional>

#include "maui/controls/application.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_safe_area_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"
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
        // (0) PRE-MOUNT hook: let the application register a handler for a user-defined control type into THIS
        //     boot's registry BEFORE the tree is walked (the generic mount resolves handlers only from the
        //     app registry, which has no global fallback). A no-op unless the app overrides on_pre_mount.
        const std::shared_ptr<maui::controls::application>& application = app.application();
        if (application != nullptr)
        {
            application->on_pre_mount(app);
        }
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
        // (4) POST-MOUNT hook: now every native view exists, run the app's post-mount demo seeding (open a
        //     SwipeView to its revealed state, drive a synthetic gesture, subscribe to the app theme, …). A
        //     no-op unless the app overrides on_post_mount.
        if (application != nullptr)
        {
            application->on_post_mount(app);
        }
    }

    maui::graphics::size drive_layout(maui::controls::window& window, double width, double height)
    {
        // Headless / origin-0 convenience: full bounds == safe-area bounds == {0,0,width,height}. A VC-backed
        // root page would arrange over the full bounds — which here equals the safe-area rect — so both
        // branches of the two-rect form collapse to the same {0,0,width,height} pass.
        const maui::graphics::rect bounds{0, 0, width, height};
        return drive_layout(window, bounds, bounds);
    }

    maui::graphics::size drive_layout(maui::controls::window& window, const maui::graphics::rect& full_bounds,
                                      const maui::graphics::rect& safe_area_bounds)
    {
        // The window host does no auto-layout (mirrors the native backends — the run loop / gallery main
        // drives the pass). Measure then arrange the content page over the chosen bounds.
        auto* page = dynamic_cast<maui::core::i_view*>(window.content());
        if (page == nullptr)
        {
            return {0, 0};
        }

        // The page ALWAYS lays out over the FULL bounds — never a pre-inset rect. MAUI insets PER VIEW, not
        // once at the host: ContentPage's SafeAreaEdges default-value creator returns None (edge-to-edge),
        // while Layout's returns Container, so the page runs under the bars/notch and its CONTENT LAYOUT is
        // what keeps the children clear of them (layout::effective_safe_area, ported from
        // MauiView.CrossPlatformArrange). Pre-insetting here instead insets the page's whole subtree —
        // which double-counts on the views that MAUI never insets (a UICollectionView applies its own
        // scroll-view insets and bypasses this chain entirely: MauiView.RespondsToSafeArea, MauiView.cs:196).
        //
        // So the host's only safe-area job is the one C# MauiView does natively: report the REALIZED insets
        // to the ISafeAreaView2 it hosts (MauiView.cs:764) and let that view decide. UIKit derives them from
        // the frame, which is not set until arrange, so the host seeds the root content layout from the rect
        // the platform already handed it — the difference between the full and safe-area rects. Headless and
        // AppKit pass the two rects equal ⇒ zero insets ⇒ every safe-area path downstream is a no-op.
        const maui::core::thickness realized_insets{
            safe_area_bounds.x - full_bounds.x, safe_area_bounds.y - full_bounds.y,
            (full_bounds.x + full_bounds.width) - (safe_area_bounds.x + safe_area_bounds.width),
            (full_bounds.y + full_bounds.height) - (safe_area_bounds.y + safe_area_bounds.height)};
        // The PAGE gets first refusal on the insets (C# MauiView.ValidateSafeArea, MauiView.cs:764-771:
        // the host stores SafeAreaInsets on the ISafeAreaView2 it hosts, and that view decides). Seeded
        // HERE rather than from the handler's safeAreaInsetsDidChange so content_page::layout_inset()
        // sees a current value on THIS pass — set_safe_area_insets is a plain property store and
        // schedules no relayout of its own.
        //
        // Whatever the page consumes is then withheld from the child push below — the port's analog of
        // C# MauiView.IsParentHandlingSafeArea (MauiView.cs:505-526), per edge, "a view never insets an
        // edge an ancestor already inset". Mac Catalyst is the only platform where the page consumes
        // anything (UseSafeArea defaults true there — ios_specific::page::use_safe_area_default); on
        // iOS/Android/headless every edge is None, child_insets == realized_insets, and this is inert.
        maui::core::thickness child_insets = realized_insets;
        if (auto* safe_area_page = dynamic_cast<maui::core::i_safe_area_view2*>(page))
        {
            safe_area_page->set_safe_area_insets(realized_insets);
            const auto consumed = [safe_area_page](int edge) {
                return safe_area_page->get_safe_area_regions_for_edge(edge) != maui::core::safe_area_regions::none;
            };
            child_insets = maui::core::thickness{
                consumed(0) ? 0.0 : realized_insets.left, consumed(1) ? 0.0 : realized_insets.top,
                consumed(2) ? 0.0 : realized_insets.right, consumed(3) ? 0.0 : realized_insets.bottom};
        }

        if (const auto* content_host = dynamic_cast<const maui::core::i_content_view*>(page))
        {
            if (auto* safe_area_content = dynamic_cast<maui::core::i_safe_area_view2*>(content_host->content()))
            {
                // iOS ONLY: reduce the pushed insets by the content's OWN MARGIN (per edge, clamped at 0).
                // The content layout is offset from the page edge by its margin (Layout.compute_frame /
                // LayoutExtensions.ComputeFrame), so that margin already provides part of the clearance from
                // the unsafe region. On iOS the status bar OVERLAYS the content (both the safe area and the
                // margin are measured from the screen edge), so pushing the FULL page-level inset on top of
                // the margin offset double-counts it — exactly what UIKit's per-view safeAreaInsets avoid (a
                // view offset N pt into its parent reports parent.safeAreaInsets - N). The port pushes a
                // single page-level inset instead of per-view, so it subtracts the content's margin here to
                // match. Measured: a <VerticalStackLayout Margin="20"> ran 20pt low on iOS before this.
                //
                // NOT on Mac Catalyst: there the titlebar is window chrome ABOVE the content area, so the
                // margin is measured from the content-area top and is ADDITIVE with the inset — MAUI renders
                // content at inset + margin, and subtracting the margin moved it 20pt too high (measured:
                // ios_blur_effect green 0.12% -> red 8.8%). The two platforms genuinely differ (ruling 1 —
                // match each platform's render); same theme as the CollectionView .Never fork. See
                // PARITY_REVIEW.md item 3. Headless (non-Apple) pushes zero insets, so this is a no-op there.
#if !defined(TARGET_OS_MACCATALYST) || !TARGET_OS_MACCATALYST
                if (const auto* content_view = dynamic_cast<const maui::core::i_view*>(content_host->content()))
                {
                    const maui::core::thickness margin = content_view->margin();
                    child_insets = maui::core::thickness{std::max(0.0, child_insets.left - margin.left),
                                                         std::max(0.0, child_insets.top - margin.top),
                                                         std::max(0.0, child_insets.right - margin.right),
                                                         std::max(0.0, child_insets.bottom - margin.bottom)};
                }
#endif
                safe_area_content->set_safe_area_insets(child_insets);
            }
        }

        page->measure(full_bounds.width, full_bounds.height);
        return page->arrange(full_bounds);
    }
} // namespace maui::hosting
