// maui::controls::shell_item — out-of-line bodies. See shell_item.hpp.

#include "maui/controls/shell/shell_item.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    shell_item::~shell_item() = default;

    void shell_item::add(std::shared_ptr<shell_section> section)
    {
        shell_section* raw = section.get();
        items_.push_back(std::move(section));
        attach_logical_child(*raw);
        // OnVisibleChildAdded: the first visible child becomes current.
        if (current_item_ == nullptr)
        {
            const std::vector<shell_section*> visible = visible_items();
            if (std::ranges::find(visible, raw) != visible.end())
            {
                set_current_item(raw);
            }
        }
        send_structure_changed();
    }

    std::shared_ptr<shell_section> shell_item::add(std::shared_ptr<shell_content> content)
    {
        std::shared_ptr<shell_section> section = shell_section::create_from_shell_content(std::move(content));
        add(section);
        return section;
    }

    std::shared_ptr<shell_section> shell_item::add(content_page& page)
    {
        return add(shell_content::adopt(page));
    }

    void shell_item::remove(const shell_section& section)
    {
        const auto it = std::ranges::find_if(items_, [&section](const std::shared_ptr<shell_section>& candidate) {
            return candidate.get() == &section;
        });
        if (it == items_.end())
        {
            return;
        }
        // Move the owning ref out before erasing the (now-emptied) slot, so the section stays alive
        // through the current-item transition without an extra copy.
        const std::shared_ptr<shell_section> removed = std::move(*it);
        items_.erase(it);
        detach_logical_child(*removed);
        if (wrapper_source_ == removed.get())
        {
            wrapper_sync_token_.reset();
            wrapper_source_ = nullptr;
        }
        // OnVisibleChildRemoved: re-select (or clear) the current item.
        if (current_item_ == removed.get())
        {
            const std::vector<shell_section*> visible = visible_items();
            set_current_item(visible.empty() ? nullptr : visible.front());
        }
        send_structure_changed();
    }

    std::vector<shell_section*> shell_item::visible_items() const
    {
        std::vector<shell_section*> visible;
        visible.reserve(items_.size());
        for (const std::shared_ptr<shell_section>& section : items_)
        {
            if (section->is_visible())
            {
                visible.push_back(section.get());
            }
        }
        return visible;
    }

    void shell_item::set_current_item(shell_section* value)
    {
        if (current_item_ == value)
        {
            return;
        }
        shell_section* old_item = current_item_;
        current_item_ = value;

        // OnCurrentItemChanged: the old section disappears first; a null new value stops there.
        if (old_item != nullptr)
        {
            old_item->send_disappearing();
        }
        if (value == nullptr)
        {
            on_property_changed("current_item");
            return;
        }
        if (dynamic_cast<shell*>(logical_parent()) != nullptr)
        {
            value->send_appearing();
        }
        if (shell* host = containing_shell(); host != nullptr && is_visible_item())
        {
            host->update_current_state(shell_navigation_source::shell_section_changed);
        }
        // (The C# TabBar QueryAttributes reset — pushing an empty parameter set into the current
        // content when switching between non-null sections — is not ported; the [QueryProperty]
        // clearing it serves is reflection-only. See i_query_attributable.hpp.)
        send_structure_changed();
        on_property_changed("current_item");
    }

    bool shell_item::is_visible_item() const
    {
        const auto* host = dynamic_cast<const shell*>(logical_parent());
        return host != nullptr && host->current_item() == this;
    }

    std::shared_ptr<shell_item> shell_item::create_from_shell_section(std::shared_ptr<shell_section> section)
    {
        // An already-parented section returns (and selects into) its existing item.
        if (auto* current = dynamic_cast<shell_item*>(section->logical_parent()))
        {
            const auto it =
                std::ranges::find_if(current->items_, [&section](const std::shared_ptr<shell_section>& candidate) {
                    return candidate == section;
                });
            if (it != current->items_.end())
            {
                current->set_current_item(section.get());
            }
            if (auto owner = current->weak_from_this().lock())
            {
                return std::static_pointer_cast<shell_item>(std::move(owner));
            }
        }

        // A Tab wraps into a TabBar, anything else into a plain ShellItem (C# CreateFromShellSection).
        std::shared_ptr<shell_item> result;
        if (dynamic_cast<tab*>(section.get()) != nullptr)
        {
            result = std::make_shared<tab_bar>();
        }
        else
        {
            result = std::make_shared<shell_item>();
        }

        result->set_route(routing::generate_implicit_route(section->route()));

        shell_section* source = section.get();
        result->add(std::move(section));

        // The C# Title/Icon/FlyoutIcon (OneWay) + FlyoutDisplayOptions (OneTime) bindings, as a live
        // sync. The token disconnects before items_ (the source's owner) is destroyed (§8 ordering).
        result->set_title(std::string{source->title()});
        result->set_icon(source->icon());
        result->set_flyout_icon(source->flyout_icon());
        result->set_flyout_display_options(source->get_flyout_display_options());
        shell_item* self = result.get();
        result->wrapper_source_ = source;
        result->wrapper_sync_token_ =
            maui::core::connect_scoped(source->property_changed, [self, source](std::string_view name) {
                if (name == "title")
                {
                    self->set_title(std::string{source->title()});
                }
                else if (name == "icon")
                {
                    self->set_icon(source->icon());
                }
                else if (name == "flyout_icon")
                {
                    self->set_flyout_icon(source->flyout_icon());
                }
            });
        return result;
    }

    void shell_item::send_structure_changed()
    {
        if (auto* host = dynamic_cast<shell*>(logical_parent()))
        {
            if (is_visible_item())
            {
                host->send_structure_changed();
            }
            host->send_flyout_items_changed();
        }
    }

    void shell_item::send_appearing()
    {
        base_shell_item::send_appearing();
        if (current_item_ != nullptr)
        {
            const auto* host = dynamic_cast<const shell*>(logical_parent());
            if (host != nullptr && host->current_item() == this)
            {
                current_item_->send_appearing();
            }
        }
    }

    void shell_item::send_disappearing()
    {
        base_shell_item::send_disappearing();
        if (current_item_ != nullptr)
        {
            current_item_->send_disappearing();
        }
    }

    shell* shell_item::containing_shell() const
    {
        return dynamic_cast<shell*>(logical_parent());
    }

    void shell_item::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const std::shared_ptr<shell_section>& section : items_)
        {
            visit(*section);
        }
    }
} // namespace maui::controls
