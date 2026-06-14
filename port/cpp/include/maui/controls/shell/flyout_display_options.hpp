#pragma once
// maui::controls::flyout_display_options  <=  Microsoft.Maui.Controls.FlyoutDisplayOptions
//
// How a shell_group_item (shell_item / shell_section) presents itself in the flyout: as one entry
// (the default) or expanded so each child gets its own entry. Ported from FlyoutDisplayOptions.cs.

namespace maui::controls
{
    enum class flyout_display_options
    {
        as_single_item,
        as_multiple_items,
    };
} // namespace maui::controls
