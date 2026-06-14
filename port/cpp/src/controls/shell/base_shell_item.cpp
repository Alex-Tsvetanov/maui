// maui::controls::base_shell_item — out-of-line bodies. See base_shell_item.hpp.

#include "maui/controls/shell/base_shell_item.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    base_shell_item::~base_shell_item()
    {
        routing::remove_route(*this);
    }

    const maui::core::bindable_property<std::string>& base_shell_item::title_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"title", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& base_shell_item::icon_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{"icon"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& base_shell_item::
        flyout_icon_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "flyout_icon"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& base_shell_item::is_enabled_descriptor()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& base_shell_item::is_checked_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_checked", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& base_shell_item::is_visible_descriptor()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_visible", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& base_shell_item::flyout_item_is_visible_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"flyout_item_is_visible", true};
        return descriptor;
    }

    void base_shell_item::set_icon(std::shared_ptr<maui::core::i_image_source> value)
    {
        // OnIconChanged: a non-null Icon seeds FlyoutIcon unless FlyoutIcon was explicitly set.
        if (value != nullptr && !flyout_icon_.is_set())
        {
            flyout_icon_.set(value);
        }
        icon_.set(std::move(value));
    }

    void base_shell_item::set_route(std::string value)
    {
        routing::validate_for_duplicates(*this, value, route_validation_siblings());
        routing::set_route(*this, std::move(value));
    }

    void base_shell_item::send_appearing()
    {
        if (has_appearing_)
        {
            return;
        }
        has_appearing_ = true;
        on_appearing_core();
        appearing.raise();
        // Drain the queued OnAppearing(Action) callbacks (one-shot, in registration order).
        const std::vector<std::function<void()>> pending = std::move(pending_appearing_);
        pending_appearing_.clear();
        for (const std::function<void()>& action : pending)
        {
            action();
        }
    }

    void base_shell_item::send_disappearing()
    {
        if (!has_appearing_)
        {
            return;
        }
        has_appearing_ = false;
        on_disappearing_core();
        disappearing.raise();
    }

    void base_shell_item::on_appearing(std::function<void()> action)
    {
        if (has_appearing_)
        {
            action();
        }
        else
        {
            pending_appearing_.push_back(std::move(action));
        }
    }

    bool base_shell_item::is_part_of_visible_tree() const
    {
        element* parent = logical_parent();
        if (auto* host = dynamic_cast<shell*>(parent))
        {
            return std::ranges::any_of(host->visible_items(), [this](const shell_item* item) { return item == this; });
        }
        if (auto* item = dynamic_cast<shell_item*>(parent))
        {
            return std::ranges::any_of(item->visible_items(),
                                       [this](const shell_section* section) { return section == this; });
        }
        if (auto* section = dynamic_cast<shell_section*>(parent))
        {
            return std::ranges::any_of(section->visible_items(),
                                       [this](const shell_content* content) { return content == this; });
        }
        return false;
    }

    shell* base_shell_item::find_parent_shell() const
    {
        for (element* parent = logical_parent(); parent != nullptr; parent = parent->logical_parent())
        {
            if (auto* host = dynamic_cast<shell*>(parent))
            {
                return host;
            }
        }
        return nullptr;
    }

    std::vector<const maui::core::bindable_object*> base_shell_item::route_validation_siblings() const
    {
        std::vector<const maui::core::bindable_object*> siblings;
        element* parent = logical_parent();
        if (auto* host = dynamic_cast<shell*>(parent))
        {
            for (const std::shared_ptr<shell_item>& item : host->items())
            {
                siblings.push_back(item.get());
            }
        }
        else if (auto* item = dynamic_cast<shell_item*>(parent))
        {
            for (const std::shared_ptr<shell_section>& section : item->items())
            {
                siblings.push_back(section.get());
            }
        }
        else if (auto* section = dynamic_cast<shell_section*>(parent))
        {
            for (const std::shared_ptr<shell_content>& content : section->items())
            {
                siblings.push_back(content.get());
            }
        }
        return siblings;
    }
} // namespace maui::controls
