#pragma once
// maui::controls::shell_navigation_source  <=  Microsoft.Maui.Controls.ShellNavigationSource
//
// How a shell navigation was initiated. Ported from ShellNavigationSource.cs (values and order
// preserved — CalculateNavigationSource and the tests key on them).

namespace maui::controls
{
    enum class shell_navigation_source
    {
        unknown = 0,
        push,
        pop,
        pop_to_root,
        insert,
        remove,
        shell_item_changed,
        shell_section_changed,
        shell_content_changed,
    };
} // namespace maui::controls
