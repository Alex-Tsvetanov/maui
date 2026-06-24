#pragma once
// maui::hosting::app_host — the GENERIC recursive mount + layout driver.
//
// No C# class maps 1:1 here: this is the port's reusable extraction of what MAUI's platform startup does
// per app (the per-platform MauiUIApplicationDelegate / MauiWinUIApplication boot) AND what the gallery
// mains hand-rolled per page (ios_gallery.mm boot_page + the per-page attach_handlers + gallery_rehost_*
// helpers). It walks an arbitrary element tree, attaches a handler to every element (children before
// parents), re-hosts each container's children into its now-attached handler, opens the window, and drives
// one measure + arrange pass — WITHOUT special-casing any control.
//
// How the generic traversal works (no per-control knowledge in this driver):
//   - element::visit_logical_children exposes the protected for_each_logical_child every container already
//     overrides (layout, content_page, border, content_view, scroll_view, …) — the tree edges.
//   - element::handler_type_tag returns the CONCRETE control's handler-registry key (the runtime analog of
//     attach_handler<View>'s static type_tag::of<View>()), so create_handler resolves the right handler
//     from a bare element&. Defaults to the implicit-style target type; the few handler-but-no-style
//     controls declare it via set_handler_type_tag<T>(). nullopt ⇒ a leaf chrome element with no handler.
//   - element::mount_into_handler re-fires each container's own host command ("set_content" / "add" / …)
//     so the native panel hosts its children — the construction-order replay (the tree is built in the
//     control ctor, before any handler exists, so the original host command found no handler).
//
// Order (mirrors the verified gallery recipe): attach handlers DEPTH-FIRST POST-ORDER (a child's native
// view exists before its parent hosts it), then re-host each container bottom-up, then attach the window
// handler, then open the window (the window handler hosts the page now that the page handler is attached),
// then measure + arrange over the window's content bounds.
//
// Stage 1 (headless): run_app's body lives in src/platform/headless/host_run.cpp and calls mount_app +
// drive_layout below; it has no native run loop, so booting + one settle pass proves the mount. The apple/
// ios run_app (Stage 2) reuses the SAME mount_app/drive_layout and adds the platform run loop + the native
// safe-area-derived bounds (see host_run.hpp).

#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class element;
    class window;
} // namespace maui::controls

namespace maui::hosting
{
    class maui_app;

    // Recursively attach a handler to `root` and its whole logical subtree (children before parents), then
    // re-host each container's children into its now-attached handler. An element whose handler_type_tag is
    // nullopt, or for which no handler is registered, is skipped (logged via the unregistered path) so one
    // unmappable control can't abort the mount — its subtree is still visited. Idempotent per element only
    // in the sense that re-running would re-attach; call once per fresh tree.
    void mount_tree(maui_app& app, maui::controls::element& root);

    // The full per-window mount: mount_tree over the window's content page, attach the window handler, and
    // open the window through the application lifecycle (which hosts the page's native view). Returns once
    // the tree is mounted and the window is open; drive_layout then sizes it.
    void mount_window(maui_app& app, maui::controls::window& window);

    // Drive one measure + arrange pass over the window's content at `width` x `height` (the content origin
    // is {0,0}; a real backend insets by the native safe area — Stage 2). Returns the page's arranged size.
    // No-op (returns {0,0}) when the window has no content page. The headless run_app calls this at a default
    // size to settle the tree; on a native backend the run loop re-drives it on every resize. Equivalent to
    // the two-rect overload below with `full_bounds == safe_area_bounds == {0, 0, width, height}` — so a
    // VC-backed root page (which always lays out over full bounds) gets the same {0,0,width,height} rect, the
    // correct headless/origin-0 behavior.
    maui::graphics::size drive_layout(maui::controls::window& window, double width, double height);

    // The two-rect form a NATIVE backend drives: measure + arrange the content page over EITHER `full_bounds`
    // (the whole controller / window) OR `safe_area_bounds` (inset past the status bar / Dynamic Island),
    // chosen by the same VC-backed-root-page contract the ios_gallery / ios host_run hand-rolled — a page
    // whose handler is an i_view_handler with a non-null root_view_controller() (flyout_page →
    // UISplitViewController, tabbed_page → UITabBarController) owns its own chrome + each inner page tracks
    // its own safe area, so it lays out over `full_bounds`; every other page lays out over `safe_area_bounds`.
    // On non-iOS backends root_view_controller() is always null, so this always picks `safe_area_bounds`
    // (pass the same rect for both for the headless/full-bounds case). Returns the page's arranged size; a
    // no-op returning {0,0} when the window has no content page.
    maui::graphics::size drive_layout(maui::controls::window& window, const maui::graphics::rect& full_bounds,
                                      const maui::graphics::rect& safe_area_bounds);
} // namespace maui::hosting
