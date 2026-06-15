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

// NOTE: swipe_item is NOT self-registered through MAUI_REGISTER_HANDLER. That seam keys on the i_element
// registry and connects via set_handler/set_virtual_view(i_element&) — but a swipe_item (: menu_item) is
// not an i_element in this port (see swipe_item_menu_item_handler.hpp). Its handler
// (maui::core::swipe_item_menu_item_handler) is a standalone handler the SwipeView creates directly when
// it materializes the swipe items natively; the tests drive set_virtual_view(swipe_item&) on it directly.
