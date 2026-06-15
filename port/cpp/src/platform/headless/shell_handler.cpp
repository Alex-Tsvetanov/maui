// shell_handler — headless platform recipe. The "native container" is the shell_render_tree mirror in
// shell_platform (built by the cross-platform rebuild()), so route → VC-stack reconfiguration is
// unit-testable with no device: the tests assert on tree.current_item_renderer (the active section + its
// vc_stack) and tree.flyout_rows. realize_tree() is a no-op here (no real VCs to build). The real twins
// are src/platform/{ios,apple}/shell_handler.mm.

#include "maui/controls/shell_handler.hpp"

#include <memory>

#include "maui/controls/shell/shell.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::core
{
    shell_platform::~shell_platform() = default;

    std::unique_ptr<shell_platform> shell_handler::create_platform_view()
    {
        return std::make_unique<shell_platform>();
    }

    void shell_handler::realize_tree()
    {
        // Headless: the shell_render_tree mirror IS the realized structure (built by rebuild()); there is
        // no native VC hierarchy to materialize.
    }

    void shell_handler::update_flyout_presented(maui::controls::shell& host)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->tree.flyout_presented = host.flyout_is_presented();
        }
    }

    void shell_handler::realize_flyout()
    {
        // Headless: the tree mirror (flyout_header / flyout_footer + header_behavior + flyout_width, filled
        // by the cross-platform rebuild()/rebuild_appearance()) IS the realized state; there is no native
        // ShellFlyoutHeaderContainer / split column to materialize.
    }

    void shell_handler::realize_search_box()
    {
        // Headless: the shell_search_box mirror IS the realized state (filled by rebuild_search_box); there
        // is no native UISearchController / NSSearchField to install.
    }

    void shell_handler::realize_appearance()
    {
        // Headless: the applied_appearance mirror IS the realized state (resolved by rebuild_appearance);
        // there is no native UINavigationBar / UITabBar / NSToolbar to tint.
    }

    void shell_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native container to position.
    }
} // namespace maui::core
