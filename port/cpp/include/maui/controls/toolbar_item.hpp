#pragma once
// maui::controls::toolbar_item  <=  Microsoft.Maui.Controls.ToolbarItem
//
// An item in a toolbar (or displayed on a page): a menu_item with a placement Order
// (Default/Primary/Secondary) and a Priority that sorts it among its page's items. Ported from
// src/Controls/src/Core/Toolbar/ToolbarItem.cs:
//   - the convenience constructor (name, icon, activated, order, priority) wires `activated` to the
//     clicked event (C# Clicked += (s,e) => activated()) and throws std::invalid_argument when the
//     action is null (C# ArgumentNullException);
//   - Order's enum makes C#'s validateValue (Default/Primary/Secondary only) unrepresentable;
//   - Priority defaults to 0; the toolbar tracker sorts by it (ToolBarItemComparer).

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/controls/toolbar_item_order.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_toolbar_item.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class toolbar_item : public menu_item, public virtual maui::core::i_toolbar_item
    {
    public:
        toolbar_item()
        {
            this->set_style_target_type<toolbar_item>();
        }

        // C# ToolbarItem(string name, string icon, Action activated, order, priority). The icon path is
        // minted into a file_image_source (C#'s implicit string → ImageSource conversion).
        toolbar_item(std::string name, const std::string& icon, std::function<void()> activated,
                     toolbar_item_order order = toolbar_item_order::default_order, int priority = 0)
            : toolbar_item()
        {
            if (!activated)
            {
                throw std::invalid_argument("toolbar_item: activated must not be null"); // C# ArgumentNullException
            }
            set_text(std::move(name));
            if (!icon.empty())
            {
                set_icon_image_source(image_source::from_file(icon));
            }
            clicked.connect([action = std::move(activated)] { action(); });
            set_order(order);
            set_priority(priority);
        }

        // Shared bindable-property descriptors (one instance per type, like ToolbarItem.*Property).
        static const maui::core::bindable_property<toolbar_item_order>& order_property();
        static const maui::core::bindable_property<int>& priority_property();

        [[nodiscard]] toolbar_item_order order() const
        {
            return order_.get();
        }
        void set_order(toolbar_item_order value)
        {
            order_.set(value);
        }

        [[nodiscard]] int priority() const
        {
            return priority_.get();
        }
        void set_priority(int value)
        {
            priority_.set(value);
        }

        // ---- i_toolbar_item ----
        [[nodiscard]] bool is_secondary() const override
        {
            return order_.get() == toolbar_item_order::secondary;
        }

    private:
        maui::core::property<toolbar_item_order> order_{*this, order_property()};
        maui::core::property<int> priority_{*this, priority_property()};
    };
} // namespace maui::controls
