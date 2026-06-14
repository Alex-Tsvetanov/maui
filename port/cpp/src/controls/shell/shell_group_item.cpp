// maui::controls::shell_group_item — out-of-line bodies. See shell_group_item.hpp.

#include "maui/controls/shell/shell_group_item.hpp"

#include "maui/controls/shell/flyout_display_options.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<flyout_display_options>& shell_group_item::flyout_display_options_property()
    {
        static const maui::core::bindable_property<flyout_display_options> descriptor{
            "flyout_display_options", flyout_display_options::as_single_item};
        return descriptor;
    }

    void shell_group_item::set_flyout_display_options(flyout_display_options value)
    {
        flyout_display_options_.set(value);
        // OnFlyoutDisplayOptionsPropertyChanged: FindParentOfType<Shell>()?.SendFlyoutItemsChanged().
        if (shell* host = find_parent_shell())
        {
            host->send_flyout_items_changed();
        }
    }
} // namespace maui::controls
