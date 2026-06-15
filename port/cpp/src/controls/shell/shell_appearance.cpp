// maui::controls::shell_appearance — out-of-line bodies. See shell_appearance.hpp.
//
// ingest() is the ShellAppearance.Ingest body: read the pivot element's Shell.* attached values (through
// shell::values_of, the side-map accessor) and fill any slot still unset HERE. Lives in its own TU rather
// than the header so it can reach shell::values_of without the header pulling in shell.hpp (which would be
// a cycle: shell.hpp already includes shell_appearance.hpp).

#include "maui/controls/shell/shell_appearance.hpp"

#include <optional>

#include "maui/controls/shell/shell.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    bool shell_appearance::ingest(const element& pivot)
    {
        const shell::appearance_values values = shell::values_of(pivot);
        bool any_set = false;

        // Fill `dest` only when it is still unset here AND the pivot set it (the C# guard
        // "_colorArray[i] == null && dataSet[i].IsSet"). Lowest pivot in the resolve walk wins per slot. The
        // destination slots are passed by reference (not an array index) so each is a compile-time member.
        const auto take = [&any_set](std::optional<maui::graphics::color>& dest,
                                     const std::optional<maui::graphics::color>& src) {
            if (!dest && src)
            {
                dest = src;
                any_set = true;
            }
        };
        take(colors_[index_background], values.background_color);
        take(colors_[index_disabled], values.disabled_color);
        take(colors_[index_foreground], values.foreground_color);
        take(colors_[index_tab_bar_background], values.tab_bar_background_color);
        take(colors_[index_tab_bar_disabled], values.tab_bar_disabled_color);
        take(colors_[index_tab_bar_foreground], values.tab_bar_foreground_color);
        take(colors_[index_tab_bar_title], values.tab_bar_title_color);
        take(colors_[index_tab_bar_unselected], values.tab_bar_unselected_color);
        take(colors_[index_title], values.title_color);
        take(colors_[index_unselected], values.unselected_color);

        if (!flyout_width_ && values.flyout_width)
        {
            flyout_width_ = values.flyout_width;
            any_set = true;
        }
        if (!flyout_height_ && values.flyout_height)
        {
            flyout_height_ = values.flyout_height;
            any_set = true;
        }

        return any_set;
    }
} // namespace maui::controls
