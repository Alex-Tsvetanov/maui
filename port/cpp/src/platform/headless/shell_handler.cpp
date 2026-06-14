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

    void shell_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native container to position.
    }
} // namespace maui::core
