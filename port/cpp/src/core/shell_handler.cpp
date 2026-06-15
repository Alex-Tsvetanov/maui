// shell_handler — cross-platform part (ShellHandler.cs mapper KEYS + the model→tree rebuild logic). The
// model→tree resolution (the current item's tab host, the per-section nav VC stacks, the flyout rows) is
// IDENTICAL on every backend, so it lives here; realize_tree() (per backend) materializes the matching
// native VC hierarchy from the built tree. The platform recipe (create + realize_tree + the generic-IView
// pushes) lives in src/platform/<backend>/shell_handler.{cpp,mm}.

#include "maui/controls/shell_handler.hpp"

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/list_proxy.hpp"
#include "maui/controls/shell/search_box_visibility.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // The shell's property mapper. Mirrors C#'s ShellHandler.Mapper (the cross-platform key set): the
    // CurrentItem / FlyoutItems / IsPresented / FlyoutBehavior keys, chained onto the shared view_mapper so
    // the generic IView properties (Visibility/Opacity/IsEnabled/AutomationId) map first.
    //
    // What triggers each key (the model→handler seam): on connect, update_properties runs EVERY key once
    // (the initial build). Post-connect, the model raises on_property_changed only for "current_item" (the
    // selection chain) and "current_state" (after navigation), plus the bindable "flyout_is_presented" /
    // "flyout_behavior" property changes. So route navigation reaches map_current_item/map_current_state →
    // rebuild (which ALSO rebuilds the flyout rows), and the flyout toggle reaches map_flyout_is_presented.
    // "flyout_items" is the C# nameof(Shell.FlyoutItems) key, exercised at connect time; the model's
    // post-connect FlyoutItemsChanged is a separate event (not a property) — a structural add/remove after
    // connect refreshes the flyout on the NEXT navigation rebuild (documented; the e2e path is navigation).
    property_mapper<i_view, shell_handler>& shell_handler::mapper()
    {
        static property_mapper<i_view, shell_handler> table{
            view_mapper(),
            {
                {"current_item", &shell_handler::map_current_item},
                {"current_state", &shell_handler::map_current_state},
                {"flyout_items", &shell_handler::map_flyout_items},
                {"flyout_is_presented", &shell_handler::map_flyout_is_presented},
                {"flyout_behavior", &shell_handler::map_flyout_behavior},
            },
        };
        return table;
    }

    command_mapper<i_view, shell_handler>& shell_handler::command_mapper()
    {
        static maui::core::command_mapper<i_view, shell_handler> table{
            {"rebuild_shell", &shell_handler::map_rebuild},
        };
        return table;
    }

    shell_handler::shell_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    void shell_handler::on_connect_handler(shell_platform& /*platform*/)
    {
    }

    void shell_handler::on_disconnect_handler(shell_platform& /*platform*/)
    {
    }

    maui::graphics::size shell_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // The shell sizes from its current page, not the handler, so it reports nothing here (the
        // container convention — cf. navigation_page_handler / flyout_page_handler).
        return {0, 0};
    }

    namespace
    {
        // Resolve a section's root content page (ShellSectionRootRenderer host): the section's current
        // content's eager-or-template page. Returns null when the section has no resolvable content.
        maui::controls::content_page* resolve_root_page(maui::controls::shell_section& section)
        {
            maui::controls::shell_content* const content = section.current_item();
            if (content == nullptr)
            {
                return nullptr;
            }
            // Prefer the already-displayed page (never creates); fall back to creating it (the template
            // case) so the renderer always has a root, exactly as ShellSectionRootRenderer requires.
            if (maui::controls::content_page* const existing = content->page())
            {
                return existing;
            }
            return content->get_or_create_content();
        }

        // Build one section's nav renderer: the root content page, then the section nav stack's pushed
        // pages (stack[1..]; slot 0 is the nullptr root marker the model keeps, ShellSection.Stack).
        maui::controls::shell_section_renderer build_section_renderer(maui::controls::shell_section& section)
        {
            maui::controls::shell_section_renderer renderer;
            renderer.section = &section;
            renderer.root_page = resolve_root_page(section);
            if (renderer.root_page != nullptr)
            {
                renderer.vc_stack.push_back(renderer.root_page);
            }
            const std::vector<maui::controls::content_page*>& stack = section.stack();
            for (std::size_t i = 1; i < stack.size(); ++i)
            {
                if (stack[i] != nullptr)
                {
                    renderer.vc_stack.push_back(stack[i]);
                }
            }
            return renderer;
        }
    } // namespace

    void shell_handler::rebuild(maui::controls::shell& host)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        // (a) The tab host for the current item (ShellItemRenderer): one section renderer per visible
        // section (a tab each); selected_index points at the item's current section.
        maui::controls::shell_item_renderer item_renderer;
        item_renderer.item = host.current_item();
        if (item_renderer.item != nullptr)
        {
            const maui::controls::shell_section* const current_section = host.current_section();
            const std::vector<maui::controls::shell_section*> visible = item_renderer.item->visible_items();
            for (maui::controls::shell_section* const section : visible)
            {
                if (section == current_section)
                {
                    item_renderer.selected_index = static_cast<int>(item_renderer.sections.size());
                }
                item_renderer.sections.push_back(build_section_renderer(*section));
            }
        }
        platform->tree.current_item_renderer = std::move(item_renderer);

        // (b) The flyout drawer rows (Shell.GetItems()).
        rebuild_flyout_rows(host);

        // Realize the matching native VC hierarchy (no-op on headless; the real twins build it).
        realize_tree();

        // (c) The current page's search box (Shell.SearchHandler) — a navigation may change the page, so
        // the box is re-resolved here (ShellPageRendererTracker.UpdateShellToMyPage on page set).
        rebuild_search_box(host);
    }

    namespace
    {
        // Fill the search-box mirror's visibility/enabled/query/results fields from the (already-resolved)
        // handler. C# UpdateSearchVisibility: Hidden removes the controller (present=false); Collapsible /
        // Expanded install it (Collapsible = HidesSearchBarWhenScrolling).
        void fill_search_box_mirror(maui::controls::shell_search_box& box, maui::controls::search_handler& found)
        {
            const maui::controls::search_box_visibility visibility = found.get_search_box_visibility();
            box.handler = &found;
            box.present = visibility != maui::controls::search_box_visibility::hidden;
            box.collapsible = visibility == maui::controls::search_box_visibility::collapsible;
            box.enabled = found.is_search_enabled();
            box.shows_results = found.shows_results();
            box.query = std::string{found.query()};
            box.placeholder = std::string{found.placeholder()};
            box.result_count = found.results() != nullptr ? found.results()->count() : 0;
        }
    } // namespace

    void shell_handler::rebuild_search_box(maui::controls::shell& host)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        // ShellPageRendererTracker reads Shell.GetSearchHandler(Page) (the current page); the port also
        // falls back to the shell itself so Shell.SetSearchHandler(shell, ...) installs a chrome-wide box.
        // Take the OWNING handle so the publisher outlives the property-changed subscription (§8) even if
        // the page's side-map entry is later cleared.
        std::shared_ptr<maui::controls::search_handler> found;
        if (const maui::controls::content_page* const page = host.current_page())
        {
            found = maui::controls::shell::get_search_handler_shared(*page);
        }
        if (found == nullptr)
        {
            found = maui::controls::shell::get_search_handler_shared(host);
        }

        maui::controls::shell_search_box& box = platform->tree.search_box;
        box = {}; // reset to "no box" before re-resolving

        // Drop the previous subscription BEFORE swapping the owning handle, so a re-resolve to a different
        // handler tears down the old token while its publisher is still alive (§8 publisher-outlives-token).
        search_box_token_.reset();
        installed_search_handler_ = found;

        if (found != nullptr)
        {
            fill_search_box_mirror(box, *found);

            // Re-realize the native box whenever the installed handler's own properties change
            // (ShellPageRendererTracker.OnSearchHandlerPropertyChanged). The lambda re-runs
            // resolve_search_box so a visibility flip to Hidden removes the box too. It captures only `this`
            // (no dangling reference to the call-scoped host): resolve_search_box reads the recorded handler.
            search_box_token_ = maui::core::connect_scoped(found->property_changed,
                                                           [this](std::string_view) { this->resolve_search_box(); });
        }

        realize_search_box();
    }

    void shell_handler::resolve_search_box()
    {
        // Re-run the resolve WITHOUT re-subscribing (we are already inside the subscription). Recompute the
        // mirror fields from the already-recorded handler and re-realize; if its visibility is now Hidden
        // the box is removed (present=false). The handler pointer is stable while the subscription lives
        // (the publisher outlives the token).
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        maui::controls::shell_search_box& box = platform->tree.search_box;
        if (box.handler == nullptr)
        {
            return;
        }
        fill_search_box_mirror(box, *box.handler);
        realize_search_box();
    }

    void shell_handler::rebuild_flyout_rows(maui::controls::shell& host)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        std::vector<maui::controls::shell_flyout_row>& rows = platform->tree.flyout_rows;
        rows.clear();
        for (maui::controls::shell_item* const item : host.visible_items())
        {
            maui::controls::shell_flyout_row row;
            row.item = item;
            row.title = std::string{item->title()};
            // C# Shell.ItemTemplate: build the row's content through the W1-09 data_template when set;
            // otherwise the native row falls back to a plain title label (per backend). The row owns the
            // created subtree; wiring its BindingContext to the item (the C# flyout-template cell context)
            // is deferred — see the header note (consistent with W2-21 leaving Shell.ItemTemplate off the
            // model). The item pointer is recorded so a future cell-binding pass can set it.
            if (flyout_item_template_ != nullptr)
            {
                row.templated_content = flyout_item_template_->create_content();
            }
            rows.push_back(std::move(row));
        }
    }

    void shell_handler::set_flyout_item_template(std::shared_ptr<maui::controls::data_template> value)
    {
        flyout_item_template_ = std::move(value);
        if (auto* host = dynamic_cast<maui::controls::shell*>(virtual_view()))
        {
            rebuild_flyout_rows(*host);
            realize_tree();
        }
    }

    // ---- the mapper entries ----

    void shell_handler::map_current_item(shell_handler& handler, i_view& view)
    {
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.rebuild(*host);
        }
    }

    void shell_handler::map_current_state(shell_handler& handler, i_view& view)
    {
        // A push/pop within the same section changes the section stack without changing current_item, so
        // the post-navigation state refresh rebuilds the renderer tree too (the new VC stacks).
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.rebuild(*host);
        }
    }

    void shell_handler::map_flyout_items(shell_handler& handler, i_view& view)
    {
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.rebuild_flyout_rows(*host);
            handler.realize_tree();
        }
    }

    void shell_handler::map_flyout_is_presented(shell_handler& handler, i_view& view)
    {
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.update_flyout_presented(*host);
        }
    }

    void shell_handler::map_flyout_behavior(shell_handler& handler, i_view& view)
    {
        // FlyoutBehavior affects how IsPresented is realized (Locked pins the drawer open), so re-realize
        // the presented state through the same path.
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.update_flyout_presented(*host);
        }
    }

    void shell_handler::map_rebuild(shell_handler& handler, i_view& view, const std::any& /*args*/)
    {
        if (auto* host = dynamic_cast<maui::controls::shell*>(&view))
        {
            handler.rebuild(*host);
        }
    }
} // namespace maui::core

// Opt-in self-registration (the OBJECT-library caveat applies — maui_core is built as usual; the handler
// TU is referenced because the shell control resolves to it). The shell self-registers its native chrome.
MAUI_REGISTER_HANDLER(maui::controls::shell, maui::core::shell_handler)
