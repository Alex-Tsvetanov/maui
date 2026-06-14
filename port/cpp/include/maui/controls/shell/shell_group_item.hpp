#pragma once
// maui::controls::shell_group_item  <=  Microsoft.Maui.Controls.ShellGroupItem
//
// The base of the grouping nodes (shell_item / shell_section): carries FlyoutDisplayOptions —
// whether the group shows as one flyout entry or expands its children. Changing it notifies the
// parent shell's flyout (ShellGroupItem.OnFlyoutDisplayOptionsPropertyChanged →
// Shell.SendFlyoutItemsChanged). Ported from ShellGroupItem.cs.

#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/flyout_display_options.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class shell_group_item : public base_shell_item
    {
    public:
        static const maui::core::bindable_property<flyout_display_options>& flyout_display_options_property();

        [[nodiscard]] flyout_display_options get_flyout_display_options() const
        {
            return flyout_display_options_.get();
        }
        void set_flyout_display_options(flyout_display_options value);

    private:
        maui::core::property<flyout_display_options> flyout_display_options_{*this, flyout_display_options_property()};
    };
} // namespace maui::controls
