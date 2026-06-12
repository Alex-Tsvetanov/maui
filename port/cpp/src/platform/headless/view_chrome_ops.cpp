// view_chrome_ops — headless platform recipe: there is no native view, so both pushes are no-ops; the
// observable state is the view_platform_base tool_tip / context_flyout mirrors the shared view_mapper
// records. The Apple twin (NSView.toolTip + NSView.menu) is src/platform/apple/view_chrome_ops.mm; the
// iOS twin (UIContextMenuInteraction attach) is src/platform/ios/view_chrome_ops.mm.

#include "maui/core/view_chrome_ops.hpp"

namespace maui::core
{
    void apply_native_tool_tip(void* /*native_view*/, const std::optional<std::string>& /*text*/)
    {
    }

    void apply_native_context_flyout(void* /*native_view*/, const i_flyout* /*flyout*/)
    {
    }
} // namespace maui::core
