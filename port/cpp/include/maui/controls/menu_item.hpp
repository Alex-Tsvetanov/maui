#pragma once
// maui::controls::menu_item  <=  Microsoft.Maui.Controls.MenuItem (over BaseMenuItem : StyleableElement)
//
// The base menu entry: a styleable element carrying Text / IsEnabled / IsDestructive / IconImageSource
// and the `clicked` activation event. Ported from src/Controls/src/Core/Menu/MenuItem.cs:
//   - activate() is IMenuItemController.Activate — the programmatic activation that raises `clicked`
//     (C# also executes Command there; the port's command channel IS the clicked event, per the W1-11
//     command-as-clicked-event collapse — Command/ICommand is not ported).
//   - the i_menu_element::clicked() inbound (what a native menu item's action invokes) routes to
//     activate(), exactly as C#'s IMenuElement.Clicked() → Activate().
//   - IsEnabled is EFFECTIVE: the explicit value AND every ancestor menu_item's enabled state — C#
//     coerces the stored value through CoerceIsEnabledProperty + PropagatePropertyChanged when a parent
//     changes; the port computes the same observable result on read (documented simplification: the
//     stored bindable keeps the explicit value, the getter walks logical_parent()).
//   - IconImageSource is stored (bindable shared_ptr); its native materialization (menu item images)
//     is deferred — see STATUS.md W1-11.

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    // i_menu_element is a VIRTUAL base: derived controls (menu_flyout_item / toolbar_item) also reach
    // it through their core contracts (i_menu_flyout_item / i_toolbar_item), and the bases must collapse
    // to one subobject for dynamic_cast<i_menu_element*> to stay unambiguous.
    class menu_item : public element, public virtual maui::core::i_menu_element
    {
    public:
        menu_item()
        {
            this->set_style_target_type<menu_item>();
        }

        // Shared bindable-property descriptors (one instance per type, like MenuItem.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<bool>& is_enabled_property();
        static const maui::core::bindable_property<bool>& is_destructive_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>&
        icon_image_source_property();

        // C# MenuItem.Clicked — raised by activate() (and so by a native menu item's action).
        maui::core::event<> clicked;

        // C# IMenuItemController.Activate(): raise `clicked` (OnClicked). C# additionally executes the
        // Command when IsEnabled — the port has no ICommand; the clicked event is the command channel.
        void activate()
        {
            clicked.raise();
        }

        // ---- i_menu_element ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
        [[nodiscard]] bool is_enabled() const override; // effective: explicit AND ancestor menus enabled
        // The inbound native activation (C# IMenuElement.Clicked() → Activate()).
        void send_clicked() override
        {
            activate();
        }

        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }

        // The EXPLICIT enabled value (C#'s _isEnabledExplicit — what the developer set, before the
        // ancestor-chain coercion the effective is_enabled() applies).
        [[nodiscard]] bool is_enabled_explicit() const
        {
            return is_enabled_.get();
        }
        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }

        [[nodiscard]] bool is_destructive() const
        {
            return is_destructive_.get();
        }
        void set_is_destructive(bool value)
        {
            is_destructive_.set(value);
        }

        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> icon_image_source() const
        {
            return icon_image_source_.get();
        }
        void set_icon_image_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            icon_image_source_.set(std::move(value));
        }

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<bool> is_enabled_{*this, is_enabled_property()};
        maui::core::property<bool> is_destructive_{*this, is_destructive_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> icon_image_source_{
            *this, icon_image_source_property()};
    };
} // namespace maui::controls
