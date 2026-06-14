// maui::controls::swipe_item — out-of-line definitions: the shared bindable-property descriptors
// (SwipeItem.BackgroundColorProperty / IsVisibleProperty). Ported from SwipeItem.cs. (The rest of the
// control is header-inline — it's a thin menu_item extension.)

#include "maui/controls/swipe_item.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::graphics::color>& swipe_item::background_color_property()
    {
        // C# SwipeItem.BackgroundColorProperty default is null (no color → nullopt via is_set()).
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"background_color"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& swipe_item::is_visible_property()
    {
        // C# SwipeItem.IsVisibleProperty default is true.
        static const maui::core::bindable_property<bool> descriptor{"is_visible", true};
        return descriptor;
    }
} // namespace maui::controls
