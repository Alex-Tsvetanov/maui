// view_chrome_ops — headless platform recipe: there is no native view, so all three pushes are no-ops;
// the observable state is the view_platform_base tool_tip / context_flyout / clip mirrors the shared
// view_mapper records. The Apple twin (NSView.toolTip + NSView.menu) is
// src/platform/apple/view_chrome_ops.mm; the iOS twin (UIContextMenuInteraction attach) is
// src/platform/ios/view_chrome_ops.mm; the Windows twin (the REAL clip push, a Composition geometric
// clip — this unit REPLACES this file in the windows build, see CMakeLists.txt's MAUI_WINDOWS_SWAPS) is
// src/platform/windows/view_chrome_ops.cpp.

#include "maui/core/view_chrome_ops.hpp"

#include <optional>
#include <string>

namespace maui::core
{
    void apply_native_tool_tip(void* /*native_view*/, const std::optional<std::string>& /*text*/)
    {
    }

    void apply_native_context_flyout(void* /*native_view*/, const i_flyout* /*flyout*/)
    {
    }

    void apply_native_clip(void* /*native_view*/, const maui::graphics::i_shape* /*shape*/)
    {
    }
} // namespace maui::core
