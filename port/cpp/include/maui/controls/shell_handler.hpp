#pragma once
// maui::core::shell_handler  <=  Microsoft.Maui.Controls.Handlers.ShellHandler /
// Microsoft.Maui.Controls.Handlers.Compatibility.ShellRenderer (iOS) + ShellItemRenderer +
// ShellSectionRenderer
//
// The NATIVE chrome behind a shell (W3-32), built over the W2-21 model (shell / shell_item /
// shell_section / shell_content + the shell_navigation_manager). The shell MODEL owns the tree and the
// URI navigation; this handler realizes it as a native container:
//
//   shell_handler          — the container view hosting (a) a pan-presented flyout drawer over (b) the
//                            current shell_item's tab host. Ported from the iOS ShellRenderer (a
//                            UIViewController owning a ShellFlyoutRenderer that wraps the current
//                            ShellItemRenderer) + the cross-platform ShellHandler mapper KEYS.
//   shell_item_renderer    — the TAB HOST for one shell_item's visible shell_sections (one tab each).
//                            Ported from ShellItemRenderer (a UITabBarController). The SELECTED tab is the
//                            shell_item's current section.
//   shell_section_renderer — the per-section NAVIGATION CONTROLLER: a VC stack whose ROOT is the
//                            section's current content's page and whose pushed entries mirror the
//                            section's nav stack (stack[1..]). Ported from ShellSectionRenderer (a
//                            UINavigationController) + ShellSectionRootRenderer (the root content host).
//
// The seam: navigation (go_to over the shell_navigation_manager) mutates the model's current
// item/section/content + the section stack, then the model raises on_property_changed("current_item") and
// ("current_state"). The view<> base routes those to handler->update_value, so map_current_item /
// map_current_state REBUILD the native renderer tree from the model — i.e. a route navigation actually
// reconfigures the native container. This mirrors C#'s ShellRenderer.OnElementPropertyChanged switching
// the current ShellItemRenderer on Shell.CurrentItem.
//
// FLYOUT: the drawer hosts one row per Shell.GetItems() visible item (the flyout item list). Each row is
// built through the flyout_item_template (a W1-09 data_template; the C# Shell.ItemTemplate /
// MenuItemTemplate seam set via set_flyout_item_template), defaulting to a plain title label per item when
// no template is set. Tapping a flyout row routes to shell::on_flyout_item_selected (the model navigates).
// DEFERRED (consistent with W2-21 leaving Shell.ItemTemplate off the model): the per-row BindingContext =
// item flow (the C# template-cell context) — the row records the created subtree + the item pointer, so a
// future cell-binding pass can wire it; this cut renders the title fallback / the raw template content.
//
// Per-backend platform recipe (the partial split, mirroring the other container handlers):
//   headless — records the RESOLVED renderer tree (shell_render_tree) so route → VC-stack
//              reconfiguration is unit-testable with no device. THIS is the structure the e2e asserts
//              (active section, the per-section VC stack).
//   ios      — a real container UIViewController whose child VCs are the flyout drawer (a
//              UISplitViewController whose primary column is the pan-presented drawer) and the current
//              item's UITabBarController; each tab is a UINavigationController (the section renderer) whose
//              viewControllers[0] is the root content. Route navigation reconfigures the real VC stack —
//              the on-simulator e2e gate.
//   apple    — an NSSplitViewController (sidebar = the flyout list, content = an NSTabView of the
//              shell_items). VISUAL DEVIATION (documented): AppKit has no pan drawer — the flyout is a
//              persistent SIDEBAR; per-section navigation collapses to swapping the content subview (no
//              NSNavigationController on macOS).

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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
    class bindable_object;
} // namespace maui::core

namespace maui::controls
{
    class shell;
    class shell_item;
    class shell_section;
    class content_page;
    class data_template;

    // The resolved per-section navigation renderer (ShellSectionRenderer + ShellSectionRootRenderer). The
    // root_page is the section's current content page; pushed_pages mirror the section nav stack[1..]. The
    // vc_stack (root first, then pushed) is the headless-observable analog of
    // UINavigationController.ViewControllers — the e2e asserts against it.
    struct shell_section_renderer
    {
        shell_section* section = nullptr;    // the model section this renderer mirrors
        content_page* root_page = nullptr;   // the section's current content page (the root VC)
        std::vector<content_page*> vc_stack; // root_page first, then the pushed pages (stack[1..])
    };

    // The resolved tab host for one shell_item (ShellItemRenderer / UITabBarController). One section
    // renderer per VISIBLE section; selected_index points at the item's current section.
    struct shell_item_renderer
    {
        shell_item* item = nullptr;                   // the model item this renderer mirrors
        std::vector<shell_section_renderer> sections; // one per visible shell_section (a tab each)
        int selected_index = -1;                      // index into sections of the current section
    };

    // One row of the flyout drawer (a Shell.GetItems() visible item rendered through the item template).
    struct shell_flyout_row
    {
        shell_item* item = nullptr;                                     // the item the row navigates to
        std::string title;                                              // the row's display title
        std::shared_ptr<maui::core::bindable_object> templated_content; // the data_template-built row (if any)
    };

    // The whole resolved renderer tree — the headless mirror of the native container. Rebuilt on every
    // current_item / current_state change (model navigation reconfigures it).
    struct shell_render_tree
    {
        shell_item_renderer current_item_renderer; // the tab host for the shell's current item
        std::vector<shell_flyout_row> flyout_rows; // the flyout drawer rows (Shell.GetItems())
        bool flyout_presented = false;             // FlyoutIsPresented as realized
    };
} // namespace maui::controls

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; the real backends override update_* on the container's view).
    struct shell_platform : view_platform_base
    {
        shell_platform() = default;
        ~shell_platform() override; // backend-defined: releases the retained native chrome on Apple/iOS
        shell_platform(const shell_platform&) = delete;
        shell_platform(shell_platform&&) = delete;
        shell_platform& operator=(const shell_platform&) = delete;
        shell_platform& operator=(shell_platform&&) = delete;

        void* native = nullptr; // the container's root view (the controller's .view on real backends)

        // The headless-observable resolved renderer tree (the native container mirror). The real backends
        // ALSO build the matching VC hierarchy; every backend keeps this mirror so the structure is
        // assertable without a device.
        maui::controls::shell_render_tree tree;

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        // The retained native chrome, shared by the two real-native twins.
        void* controller = nullptr;    // the container UIViewController / NSSplitViewController (retained)
        void* tab_host = nullptr;      // the current item's UITabBarController / the content NSTabView (retained)
        void* flyout_host = nullptr;   // the flyout drawer column / sidebar wrapper controller (retained)
        void* section_hosts = nullptr; // a retained array of the per-section nav controllers / wrappers
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the split controller's NSView (defined in
        // src/platform/apple/shell_handler.mm). is_enabled keeps the base mirror (plain NSView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the container's UIView (defined in
        // src/platform/ios/shell_handler.mm). is_enabled keeps the base mirror (plain UIView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class shell_handler : public view_handler<shell_handler, i_view, shell_platform>
    {
    public:
        shell_handler();

        static property_mapper<i_view, shell_handler>& mapper();
        static command_mapper<i_view, shell_handler>& command_mapper();

        static std::unique_ptr<shell_platform> create_platform_view();

        // C# OnConnectHandler: nothing platform-specific to wire here beyond create_platform_view (the
        // headless build defines it empty; the real twins use it to point any retained delegate at this).
        void on_connect_handler(shell_platform& platform);
        void on_disconnect_handler(shell_platform& platform);

        // The shell computes its size from its current page, not the handler (the container convention).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        // Frame the container's root view to the given frame.
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Rebuild the whole resolved renderer tree (tab host + section renderers + flyout rows) from the
        // model's CURRENT item/section/content + section stacks. CROSS-PLATFORM (shell_handler.cpp): pure
        // model→tree logic, identical on every backend. After the tree is rebuilt it calls realize_tree()
        // so the real twins materialize the matching native VC hierarchy. Called by map_current_item /
        // map_current_state (and on connect via the property mapper).
        void rebuild(maui::controls::shell& host);
        // Materialize the native VC hierarchy from the already-built shell_render_tree mirror (defined per
        // backend: headless no-op; iOS/AppKit build the real container/tab/nav controllers). Separated
        // from rebuild so the model→tree logic stays cross-platform.
        void realize_tree();
        // Realize FlyoutIsPresented natively (drawer slide on iOS; sidebar collapse on AppKit; mirror
        // everywhere). Defined per backend.
        void update_flyout_presented(maui::controls::shell& host);

        // The flyout item template (C# Shell.ItemTemplate) — the W1-09 data_template each flyout row is
        // built through. A null template falls back to a plain title label per item. Setting it rebuilds.
        void set_flyout_item_template(std::shared_ptr<maui::controls::data_template> value);
        [[nodiscard]] const std::shared_ptr<maui::controls::data_template>& flyout_item_template() const
        {
            return flyout_item_template_;
        }

        // ---- the mapper entries (C# ShellHandler.Mapper) ----
        // "current_item" → C# MapCurrentItem: rebuild the tab host for the new current item.
        static void map_current_item(shell_handler& handler, i_view& view);
        // "current_state" → the post-navigation refresh: the section stacks may have changed without the
        // current item changing (a push/pop within the same section), so rebuild from the new state.
        static void map_current_state(shell_handler& handler, i_view& view);
        // "flyout_items" → C# MapFlyoutItems: rebuild the flyout drawer rows (run at connect time; a
        // navigation rebuild also refreshes them).
        static void map_flyout_items(shell_handler& handler, i_view& view);
        // "flyout_is_presented" → C# MapIsPresented: realize the presented state.
        static void map_flyout_is_presented(shell_handler& handler, i_view& view);
        // "flyout_behavior" → C# MapFlyoutBehavior: realize the behavior (mirror; affects presentation).
        static void map_flyout_behavior(shell_handler& handler, i_view& view);

        // The "rebuild_shell" COMMAND — the control re-issues this after wiring so a freshly-attached
        // container realizes the current item (C# ShellRenderer.SetupCurrentShellItem on ViewDidLoad).
        static void map_rebuild(shell_handler& handler, i_view& view, const std::any& args);

    private:
        // Read the model's flyout rows (Shell.GetItems()) into the tree, building each row through the
        // item template when set (cross-platform — the tree mirror is shared; the real twins read it to
        // build native rows). Returns the rows so the .mm can materialize them.
        void rebuild_flyout_rows(maui::controls::shell& host);

        std::shared_ptr<maui::controls::data_template> flyout_item_template_;
    };
} // namespace maui::core
