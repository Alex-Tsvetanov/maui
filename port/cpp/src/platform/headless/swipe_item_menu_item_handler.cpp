// swipe_item_menu_item_handler — headless platform recipe. There is no native UIButton; the "native
// host" is the swipe_item_menu_item_platform mirror (title / title colour / background / font / source /
// visibility) so tests can observe what the native button would track as each property is mapped. The iOS
// twin (a real UIButton subclass + a frame-observer proxy) is
// src/platform/ios/swipe_item_menu_item_handler.mm.

#include "maui/core/swipe_item_menu_item_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::core
{
    swipe_item_menu_item_platform::~swipe_item_menu_item_platform() = default;

    std::unique_ptr<swipe_item_menu_item_platform> swipe_item_menu_item_handler::create_platform_view()
    {
        return std::make_unique<swipe_item_menu_item_platform>();
    }

    // C# ConnectHandler / SwipeItemButtonProxy.Connect — headless: no UIButton frame to observe.
    void swipe_item_menu_item_handler::connect()
    {
    }

    // C# DisconnectHandler / SwipeItemButtonProxy.Disconnect — headless: nothing to tear down.
    void swipe_item_menu_item_handler::disconnect() const
    {
    }

    // C# MapText: RestorationIdentifier = Text; SetTitle(Text). The mirror records the title.
    void swipe_item_menu_item_handler::apply_text() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->title = std::string(item_view_->text());
    }

    // C# MapTextColor: SetTitleColor(view.GetTextColor()) — the luminosity-derived effective colour (null
    // leaves the title colour untouched, matching C#'s `if (color != null)` guard).
    void swipe_item_menu_item_handler::apply_text_color() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto color = get_text_color(*item_view_);
        platform->has_title_color = color.has_value();
        if (color.has_value())
        {
            platform->title_color_argb = color->to_uint();
        }
    }

    // C# MapCharacterSpacing: UpdateCharacterSpacing(view). The mirror records the value.
    void swipe_item_menu_item_handler::apply_character_spacing() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->character_spacing = item_view_->character_spacing();
    }

    // C# MapFont: UpdateFont(view, fontManager). The mirror records the font.
    void swipe_item_menu_item_handler::apply_font() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->item_font = item_view_->font();
    }

    // C# MapBackground: UpdateBackground(view.Background). The mirror records whether a paint was applied
    // and its colour.
    void swipe_item_menu_item_handler::apply_background() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::graphics::paint* const paint = item_view_->background();
        platform->has_background = paint != nullptr;
        if (paint != nullptr)
        {
            platform->background_argb = paint->background_color().to_uint();
        }
    }

    // C# MapSource: load + resize the icon. Headless has no UIImage to resize; the mirror records whether
    // a non-empty source was set.
    void swipe_item_menu_item_handler::apply_source() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto source = item_view_->source();
        platform->has_source = source != nullptr && !source->is_empty();
    }

    // C# MapVisibility: notify the parent MauiSwipeView (UpdateIsVisibleSwipeItem) + UpdateVisibility.
    // Headless has no parent native view to walk to; the mirror records the visibility.
    void swipe_item_menu_item_handler::apply_visibility() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->item_visibility = item_view_->visibility();
    }
} // namespace maui::core
