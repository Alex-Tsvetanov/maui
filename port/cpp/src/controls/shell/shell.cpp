// maui::controls::shell — out-of-line bodies. See shell.hpp.

#include "maui/controls/shell/shell.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/route_request_builder.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_navigated_event_args.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    shell::shell()
    {
        this->set_style_target_type<shell>();
        routing::set_route(*this, routing::generate_implicit_route("shell"));

        // The C# ctor wiring: the manager's events surface as the shell's own (SendNavigating /
        // SendNavigated raise the public event, then the protected virtual hook).
        navigating_token_ =
            maui::core::connect_scoped(navigation_manager_.navigating, [this](shell_navigating_event_args& args) {
                navigating.raise(args);
                on_navigating(args);
            });
        navigated_token_ =
            maui::core::connect_scoped(navigation_manager_.navigated, [this](const shell_navigated_event_args& args) {
                navigated.raise(args);
                on_navigated(args);
            });
    }

    shell::~shell()
    {
        routing::remove_route(*this); // the side-map hygiene (see routing.hpp)
    }

    const maui::core::bindable_property<flyout_behavior>& shell::flyout_behavior_property()
    {
        static const maui::core::bindable_property<flyout_behavior> descriptor{"flyout_behavior",
                                                                               flyout_behavior::flyout};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& shell::flyout_is_presented_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"flyout_is_presented", false};
        return descriptor;
    }

    // ---- Items ----

    void shell::add_item(std::shared_ptr<shell_item> item)
    {
        shell_item* raw = item.get();
        items_.push_back(std::move(item));
        attach_logical_child(*raw);
        auto_select_current_item();
        send_structure_changed();
        send_flyout_items_changed();
    }

    std::shared_ptr<shell_item> shell::add_item(std::shared_ptr<shell_section> section)
    {
        std::shared_ptr<shell_item> item = shell_item::create_from_shell_section(std::move(section));
        if (std::ranges::find(items_, item) == items_.end())
        {
            add_item(item);
        }
        return item;
    }

    std::shared_ptr<shell_item> shell::add_item(std::shared_ptr<shell_content> content)
    {
        return add_item(shell_section::create_from_shell_content(std::move(content)));
    }

    std::shared_ptr<shell_item> shell::add_item(content_page& page)
    {
        return add_item(shell_content::adopt(page));
    }

    void shell::remove_item(const shell_item& item)
    {
        const auto it = std::ranges::find_if(
            items_, [&item](const std::shared_ptr<shell_item>& candidate) { return candidate.get() == &item; });
        if (it == items_.end())
        {
            return;
        }
        // Move the owning ref out before erasing the (now-emptied) slot, so the item stays alive
        // through the selection transition without an extra copy.
        const std::shared_ptr<shell_item> removed = std::move(*it);
        items_.erase(it);
        detach_logical_child(*removed);
        if (current_item_ == removed.get())
        {
            current_item_ = nullptr; // pointer hygiene before re-selection (C# leans on the GC)
        }
        auto_select_current_item();
        send_structure_changed();
        send_flyout_items_changed();
    }

    std::vector<shell_item*> shell::visible_items() const
    {
        std::vector<shell_item*> visible;
        visible.reserve(items_.size());
        for (const std::shared_ptr<shell_item>& item : items_)
        {
            if (item->is_visible())
            {
                visible.push_back(item.get());
            }
        }
        return visible;
    }

    void shell::auto_select_current_item()
    {
        // Shell.Initialize's SetCurrentItem: keep a still-visible current item; otherwise select the
        // first valid item. (The C# hot-reload "renavigate to CurrentState" branch is not ported.)
        const std::vector<shell_item*> visible = visible_items();
        if (current_item_ != nullptr && std::ranges::find(visible, current_item_) != visible.end())
        {
            return;
        }
        if (!visible.empty())
        {
            on_flyout_item_selected(*visible.front(), false);
        }
    }

    // ---- CurrentItem ----

    void shell::set_current_item(const std::shared_ptr<shell_item>& item)
    {
        set_current_item_core(item);
    }

    void shell::set_current_item(const std::shared_ptr<shell_section>& section)
    {
        set_current_item_core(shell_item::create_from_shell_section(section));
    }

    void shell::set_current_item(const std::shared_ptr<shell_content>& content)
    {
        set_current_item_core(shell_item::create_from_shell_section(shell_section::create_from_shell_content(content)));
    }

    void shell::set_current_item(content_page& page)
    {
        set_current_item_core(shell_item::create_from_shell_section(
            shell_section::create_from_shell_content(shell_content::adopt(page))));
    }

    void shell::set_current_item_core(const std::shared_ptr<shell_item>& item)
    {
        if (item == nullptr || item.get() == current_item_)
        {
            return;
        }

        // OnCurrentItemChanging: a missing item joins the collection first, then the change is
        // proposed (canCancel: false — a direct CurrentItem set cannot be cancelled).
        if (std::ranges::find(items_, item) == items_.end())
        {
            shell_item* raw = item.get();
            items_.push_back(item);
            attach_logical_child(*raw);
            send_structure_changed();
            send_flyout_items_changed();
        }
        shell_section* section = item->current_item();
        shell_content* content = section != nullptr ? section->current_item() : nullptr;
        (void)navigation_manager_.propose_navigation_outside_goto(shell_navigation_source::shell_item_changed,
                                                                  item.get(), section, content,
                                                                  section != nullptr ? &section->stack() : nullptr,
                                                                  /*can_cancel=*/false, /*is_animated=*/true);

        // OnCurrentItemChanged.
        shell_item* old_item = current_item_;
        current_item_ = item.get();
        if (old_item != nullptr)
        {
            old_item->send_disappearing();
        }
        current_item_->send_appearing();
        update_checked();
        update_current_state(shell_navigation_source::shell_item_changed);
        on_property_changed("current_item");
    }

    std::shared_ptr<shell_item> shell::owner_of(const shell_item* item) const
    {
        for (const std::shared_ptr<shell_item>& candidate : items_)
        {
            if (candidate.get() == item)
            {
                return candidate;
            }
        }
        return nullptr;
    }

    shell_section* shell::current_section() const
    {
        return current_item_ != nullptr ? current_item_->current_item() : nullptr;
    }

    shell_content* shell::current_content() const
    {
        const shell_section* section = current_section();
        return section != nullptr ? section->current_item() : nullptr;
    }

    content_page* shell::current_page() const
    {
        const shell_section* section = current_section();
        return section != nullptr ? section->presented_page() : nullptr;
    }

    // ---- navigation ----

    void shell::go_to_async(const shell_navigation_state& state, std::optional<bool> animate,
                            std::optional<shell_route_parameters> parameters)
    {
        navigation_manager_.go_to(state, animate, false, nullptr, std::move(parameters));
    }

    const std::vector<content_page*>& shell::navigation_stack() const
    {
        static const std::vector<content_page*> empty;
        const shell_section* section = current_section();
        return section != nullptr ? section->stack() : empty;
    }

    void shell::navigation_push(content_page& page, std::optional<bool> animated)
    {
        shell_section* section = current_section();
        if (section == nullptr)
        {
            return;
        }
        if (!section->is_visible_section())
        {
            section->on_push(page, animated.value_or(true));
            return;
        }
        shell_navigation_parameters navigation_parameters;
        navigation_parameters.animated = animated;
        navigation_parameters.page_pushing = &page;
        navigation_manager_.go_to(std::move(navigation_parameters));
    }

    void shell::navigation_pop(std::optional<bool> animated)
    {
        shell_section* section = current_section();
        if (section == nullptr)
        {
            return;
        }
        if (!section->is_visible_section())
        {
            (void)section->on_pop(animated.value_or(true));
            return;
        }
        shell_navigation_parameters navigation_parameters;
        navigation_parameters.animated = animated;
        navigation_parameters.target_state = shell_navigation_state{".."};
        navigation_manager_.go_to(std::move(navigation_parameters));
    }

    void shell::navigation_pop_to_root(std::optional<bool> animated)
    {
        shell_section* section = current_section();
        if (section == nullptr)
        {
            return;
        }
        if (!section->is_visible_section())
        {
            section->on_pop_to_root(animated.value_or(true));
            return;
        }
        shell_navigation_parameters navigation_parameters;
        navigation_parameters.animated = animated;
        navigation_parameters.target_state = shell_navigation_manager::get_navigation_state(
            current_item_, section, section->current_item(), nullptr, nullptr);
        navigation_parameters.pop_all_pages_not_specified_on_target_state = true;
        navigation_manager_.go_to(std::move(navigation_parameters));
    }

    void shell::navigation_remove_page(content_page& page)
    {
        shell_section* section = current_section();
        if (section == nullptr)
        {
            return;
        }
        if (!section->is_visible_section() || navigation_manager_.accumulate_navigated_events())
        {
            section->on_remove_page(page);
            return;
        }

        // NavigationImpl.OnRemovePage: announce the (uncancellable) Navigating, mutate, update state.
        std::vector<content_page*> stack = section->stack();
        std::erase(stack, &page);
        const shell_navigation_state target = shell_navigation_manager::get_navigation_state(
            current_item_, section, section->current_item(), &stack, nullptr);

        auto args =
            std::make_shared<shell_navigating_event_args>(current_state_, target, shell_navigation_source::remove,
                                                          /*can_cancel=*/false);
        navigation_manager_.handle_navigating(*args);
        section->on_remove_page(page);
        update_current_state(shell_navigation_source::remove);
    }

    void shell::navigation_insert_page_before(content_page& page, content_page& before)
    {
        shell_section* section = current_section();
        if (section == nullptr)
        {
            return;
        }
        if (!section->is_visible_section() || navigation_manager_.accumulate_navigated_events())
        {
            section->on_insert_page_before(page, before);
            return;
        }

        std::vector<content_page*> stack = section->stack();
        const auto it = std::ranges::find(stack, &before);
        if (it == stack.end())
        {
            throw std::invalid_argument{"Page not found in nav stack"};
        }
        stack.insert(it, &page);
        const shell_navigation_state target = shell_navigation_manager::get_navigation_state(
            current_item_, section, section->current_item(), &stack, nullptr);

        auto args =
            std::make_shared<shell_navigating_event_args>(current_state_, target, shell_navigation_source::insert,
                                                          /*can_cancel=*/false);
        navigation_manager_.handle_navigating(*args);
        section->on_insert_page_before(page, before);
        update_current_state(shell_navigation_source::insert);
    }

    std::string shell::route() const
    {
        return routing::get_route(*this);
    }

    // ---- IShellController ----

    bool shell::propose_navigation(shell_navigation_source source, shell_item* item, shell_section* section,
                                   shell_content* content, const std::vector<content_page*>* stack, bool can_cancel)
    {
        return navigation_manager_.propose_navigation_outside_goto(source, item, section, content, stack, can_cancel,
                                                                   /*is_animated=*/true);
    }

    void shell::update_current_state(shell_navigation_source source)
    {
        shell_item* item = current_item_;
        shell_section* section = item != nullptr ? item->current_item() : nullptr;
        shell_content* content = section != nullptr ? section->current_item() : nullptr;
        const shell_navigation_state result = shell_navigation_manager::get_navigation_state(
            item, section, content, section != nullptr ? &section->stack() : nullptr, nullptr);

        if (!current_state_ || result.location() != current_state_->location())
        {
            const std::optional<shell_navigation_state> old_state = current_state_;
            current_state_ = result;
            on_property_changed("current_state");
            // (the C# NavigatingFrom page-event replay is page-level plumbing the port doesn't model)
            navigation_manager_.handle_navigated(shell_navigated_event_args{old_state, result, source});
        }
    }

    void shell::on_flyout_item_selected(base_shell_item& element)
    {
        on_flyout_item_selected(element, true);
    }

    void shell::on_flyout_item_selected(base_shell_item& element, bool platform_initiated)
    {
        shell_item* item = nullptr;
        shell_section* section = nullptr;
        shell_content* content = nullptr;

        if (auto* as_item = dynamic_cast<shell_item*>(&element))
        {
            item = as_item;
        }
        else if (auto* as_section = dynamic_cast<shell_section*>(&element))
        {
            item = as_section->parent_item();
            section = as_section;
        }
        else if (auto* as_content = dynamic_cast<shell_content*>(&element))
        {
            section = dynamic_cast<shell_section*>(as_content->logical_parent());
            item = section != nullptr ? section->parent_item() : nullptr;
            content = as_content;
        }

        if (item == nullptr || !item->is_enabled())
        {
            return;
        }

        section = section != nullptr ? section : item->current_item();
        content = content != nullptr ? content : (section != nullptr ? section->current_item() : nullptr);

        if (platform_initiated && flyout_is_presented() && effective_flyout_behavior() != flyout_behavior::locked)
        {
            set_flyout_is_presented(false);
        }

        if (section == nullptr || content == nullptr)
        {
            // C# waits for the item's CurrentItem to materialize via PropertyChanged; the port's
            // tree wires current items synchronously on add, so an empty chain is simply not
            // navigable yet.
            return;
        }

        if (current_item_ == nullptr)
        {
            const shell_navigation_state state =
                shell_navigation_manager::get_navigation_state(item, section, content, &section->stack(), nullptr);

            route_request_builder request_builder{
                std::vector<std::string>{item->route(), section->route(), content->route()}};
            shell_uri_handler::node_location node;
            node.set_node(search_node{content});
            (void)request_builder.add_match(node);

            auto nav_request = std::make_shared<shell_navigation_request>(
                request_definition{request_builder, *this}, shell_uri_handler::stack_request_kind::replace_it,
                std::string{}, std::string{});

            shell_navigation_parameters navigation_parameters;
            navigation_parameters.target_state = state;
            navigation_parameters.animated = false;
            navigation_manager_.go_to(std::move(navigation_parameters), std::move(nav_request));
        }
        else
        {
            navigation_manager_.go_to(shell_navigation_manager::get_navigation_parameters(item, section, content,
                                                                                          &section->stack(), nullptr));
        }
    }

    void shell::update_checked()
    {
        // Shell.UpdateChecked(root): the current chain is checked, everything else unchecked.
        const std::function<void(base_shell_item&, bool)> update = [&update](base_shell_item& root, bool checked) {
            if (!checked && !root.is_checked())
            {
                return;
            }
            root.set_is_checked(checked);
            if (auto* item = dynamic_cast<shell_item*>(&root))
            {
                for (shell_section* section : item->visible_items())
                {
                    update(*section, checked && section == item->current_item());
                }
            }
            else if (auto* section = dynamic_cast<shell_section*>(&root))
            {
                for (shell_content* content : section->visible_items())
                {
                    update(*content, checked && content == section->current_item());
                }
            }
        };
        for (shell_item* item : visible_items())
        {
            update(*item, item == current_item_);
        }
    }

    void shell::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const std::shared_ptr<shell_item>& item : items_)
        {
            visit(*item);
        }
    }
} // namespace maui::controls
