// swipe_item_menu_item_handler — cross-platform part: the ctor, the connect/update lifecycle, and the
// SwipeViewExtensions.GetTextColor helper. The platform recipe (create the native button, push title/
// colour/font/background/icon, the frame observer) lives in the per-backend partial. Ported from
// SwipeItemMenuItemHandler.cs and SwipeViewExtensions.cs.
//
// STANDALONE handler (see swipe_item_menu_item_handler.hpp): the port's i_swipe_item_menu_item is not an
// i_element, so this is not an i_element_handler and does not use the generic property_mapper — it pushes
// the same properties (the apply_* mirror MapText/MapTextColor/...) directly, and re-pushes one property
// from update_value(name).

#include "maui/core/swipe_item_menu_item_handler.hpp"

#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::core
{
    // Microsoft.Maui.Platform.SwipeViewExtensions.GetTextColor: returns nullopt when there is no
    // background colour, or when the icon is a font-image source carrying its own colour (the icon tint
    // then drives the appearance); otherwise white on a dark background / black on a light one, using the
    // C# luminosity coefficients against the 0.75 threshold.
    std::optional<maui::graphics::color> get_text_color(const i_swipe_item_menu_item& item)
    {
        const maui::graphics::paint* const paint = item.background();
        if (paint == nullptr)
        {
            return std::nullopt;
        }
        const std::shared_ptr<maui::core::i_image_source> source = item.source();
        if (const auto* font_source = dynamic_cast<const maui::core::i_font_image_source*>(source.get());
            font_source != nullptr && font_source->color().alpha > 0.0F)
        {
            return std::nullopt;
        }

        const maui::graphics::color background = paint->background_color();
        const float luminosity =
            (0.2126F * background.red) + (0.7152F * background.green) + (0.0722F * background.blue);
        return luminosity < 0.75F ? maui::graphics::colors::white : maui::graphics::colors::black;
    }

    swipe_item_menu_item_handler::swipe_item_menu_item_handler() = default;

    // Defined here (not =default in the header) so the swipe_item_menu_item_platform's backend destructor
    // is reachable — the unique_ptr member needs the complete type at the point of destruction.
    swipe_item_menu_item_handler::~swipe_item_menu_item_handler() = default;

    // ElementHandler.SetVirtualView: create the native button on first connect, wire the frame observer,
    // then push every mapped property. Re-running with the same item is a no-op.
    void swipe_item_menu_item_handler::set_virtual_view(i_swipe_item_menu_item& item)
    {
        if (item_view_ == &item)
        {
            return;
        }
        const bool first_setup = (item_view_ == nullptr);
        item_view_ = &item;

        if (!platform_view_)
        {
            platform_view_ = create_platform_view();
        }
        if (platform_view_)
        {
            platform_view_->hosted_item = item_view_;
        }
        if (first_setup && platform_view_)
        {
            connect(); // wire the frame observer (SwipeItemButtonProxy)
        }
        apply_all();
    }

    // C# UpdateValue: re-push one property by its snake_case key. Unknown keys are ignored.
    void swipe_item_menu_item_handler::update_value(std::string_view property)
    {
        if (item_view_ == nullptr)
        {
            return;
        }
        if (property == "text")
        {
            apply_text();
        }
        else if (property == "text_color")
        {
            apply_text_color();
        }
        else if (property == "character_spacing")
        {
            apply_character_spacing();
        }
        else if (property == "font")
        {
            apply_font();
        }
        else if (property == "background")
        {
            apply_background();
        }
        else if (property == "source")
        {
            apply_source();
        }
        else if (property == "visibility")
        {
            apply_visibility();
        }
    }

    // ElementHandler.DisconnectHandler: tear down the frame observer through the (still-live) platform
    // button, then drop the reference. Idempotent — a no-op once disconnected.
    void swipe_item_menu_item_handler::disconnect_handler()
    {
        if (platform_view_ && item_view_ != nullptr)
        {
            disconnect();
            const std::unique_ptr<swipe_item_menu_item_platform> old = std::move(platform_view_);
            item_view_ = nullptr;
        }
    }

    // SetVirtualView's mapper pass — push every property once, in the SwipeItemMenuItemHandler.Mapper
    // order (Visibility / Background / Text / TextColor / CharacterSpacing / Font / Source).
    void swipe_item_menu_item_handler::apply_all() const
    {
        apply_visibility();
        apply_background();
        apply_text();
        apply_text_color();
        apply_character_spacing();
        apply_font();
        apply_source();
    }
} // namespace maui::core
