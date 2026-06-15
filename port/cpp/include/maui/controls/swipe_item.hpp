#pragma once
// maui::controls::swipe_item  <=  Microsoft.Maui.Controls.SwipeItem
//
// A menu item displayed in a SwipeView when the view is swiped. Ported from
// src/Controls/src/Core/SwipeView/SwipeItem.cs (SwipeItem : MenuItem, Controls.ISwipeItem,
// Maui.ISwipeItemMenuItem): it adds a BackgroundColor (default null) and IsVisible (default true) over
// the menu_item base, the `invoked` event, and OnInvoked() — which executes the command then raises
// Invoked. Since the port has no ICommand (the command channel IS the clicked event, per the W1-11
// collapse), OnInvoked() activates the menu item (OnClicked → `clicked`) and raises `invoked`, matching
// the observable C# sequence.
//
// The ISwipeItemMenuItem faces map straight off the bindable surface: Background => SolidPaint over the
// BackgroundColor (null when unset → the state machine treats it as no background); Visibility =>
// IsVisible ? Visible : Collapsed (the swipe state machine's GetIsVisible reads this to skip collapsed
// items). AutomationId is carried here (C# Element.AutomationId; menu_item doesn't model it) for the
// native item identification — behaviorally inert at this layer. The text-style face + Source come from
// the inert MenuItem.ITextStyle defaults (TextColor null → black, Font.Default, CharacterSpacing 0 —
// MenuItem.cs:156-160) and the menu_item icon (IMenuElement.Source => IconImageSource).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/menu_item.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/property.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    class swipe_item : public menu_item, public maui::core::i_swipe_item_menu_item
    {
    public:
        swipe_item()
        {
            this->set_style_target_type<swipe_item>();
        }

        // Shared bindable-property descriptors (SwipeItem.BackgroundColorProperty / IsVisibleProperty).
        static const maui::core::bindable_property<maui::graphics::color>& background_color_property();
        static const maui::core::bindable_property<bool>& is_visible_property();

        // C# SwipeItem.Invoked — raised by on_invoked().
        maui::core::event<> invoked;

        // ---- BackgroundColor (C# default null → nullopt) ----
        [[nodiscard]] std::optional<maui::graphics::color> background_color() const
        {
            if (!background_color_.is_set())
            {
                return std::nullopt;
            }
            return background_color_.get();
        }
        void set_background_color(maui::graphics::color value)
        {
            background_color_.set(value);
        }

        // ---- IsVisible (C# default true) ----
        [[nodiscard]] bool is_visible() const
        {
            return is_visible_.get();
        }
        void set_is_visible(bool value)
        {
            is_visible_.set(value);
        }

        // ---- AutomationId (C# Element.AutomationId; stored here — see header) ----
        void set_automation_id(std::string value)
        {
            automation_id_ = std::move(value);
        }

        // ---- i_swipe_item ----
        [[nodiscard]] std::string_view automation_id() const override
        {
            return automation_id_;
        }
        // C# SwipeItem.ISwipeItem.OnInvoked: execute the command (collapsed to the clicked channel via
        // activate → OnClicked), then raise Invoked.
        void on_invoked() override
        {
            activate();
            invoked.raise();
        }

        // ---- i_swipe_item_menu_item ----
        // C# SwipeItem.ISwipeItemMenuItem.Background => new SolidPaint(BackgroundColor). Materialized
        // lazily into background_paint_ so the returned pointer stays valid; null when no color is set
        // (C# always returns a SolidPaint, but with a null color the paint is inert — the port returns
        // null so the state machine's "no background" branch is reachable).
        [[nodiscard]] const maui::graphics::paint* background() const override
        {
            const auto color = background_color();
            if (!color.has_value())
            {
                background_paint_.reset();
                return nullptr;
            }
            background_paint_ = std::make_unique<maui::graphics::solid_paint>(*color);
            return background_paint_.get();
        }

        // C# SwipeItem.ISwipeItemMenuItem.Visibility => IsVisible ? Visible : Collapsed.
        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return is_visible() ? maui::core::visibility::visible : maui::core::visibility::collapsed;
        }

        // ---- i_text_style (the inert MenuItem.ITextStyle defaults — MenuItem.cs:156-160) ----
        // C# MenuItem.ITextStyle.TextColor => null. The port has no nullable colour on the contract; the
        // default-constructed colour (opaque black) is the documented stand-in (the visible label colour
        // is driven by get_text_color() off the background, not this).
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return {};
        }
        // C# MenuItem.ITextStyle.Font => Font.Default.
        [[nodiscard]] maui::core::font font() const override
        {
            return maui::core::font::default_font();
        }
        // C# MenuItem.ITextStyle.CharacterSpacing => 0.
        [[nodiscard]] double character_spacing() const override
        {
            return 0;
        }

        // C# SwipeItem.IMenuElement.Source => IconImageSource (carried by the menu_item base).
        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> source() const override
        {
            return icon_image_source();
        }

    private:
        maui::core::property<maui::graphics::color> background_color_{*this, background_color_property()};
        maui::core::property<bool> is_visible_{*this, is_visible_property()};
        std::string automation_id_;
        // Lazily-built SolidPaint backing the borrowed background() pointer (see above). Mutable: a
        // const-qualified getter rebuilds it on demand.
        mutable std::unique_ptr<maui::graphics::solid_paint> background_paint_;
    };
} // namespace maui::controls
