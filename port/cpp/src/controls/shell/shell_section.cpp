// maui::controls::shell_section — out-of-line bodies. See shell_section.hpp.
//
// The navigation paths are ported from ShellSection.cs with the Task pipeline collapsed to
// synchronous (no handler this unit, so InvokeNavigationRequest / the completion sources vanish)
// and every MODAL branch removed (the port's shell has no modal stack — STATUS.md).

#include "maui/controls/shell/shell_section.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigation_manager.hpp"
#include "maui/controls/shell/shell_navigation_request.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    shell_section::~shell_section() = default;

    void shell_section::add(std::shared_ptr<shell_content> content)
    {
        shell_content* raw = content.get();
        items_.push_back(std::move(content));
        attach_logical_child(*raw);
        // OnVisibleChildAdded: the first visible child becomes current; any current refreshes the page.
        if (current_item_ == nullptr)
        {
            const std::vector<shell_content*> visible = visible_items();
            if (std::ranges::find(visible, raw) != visible.end())
            {
                set_current_item(raw);
            }
        }
        if (current_item_ != nullptr)
        {
            update_displayed_page();
        }
        send_structure_changed();
    }

    std::shared_ptr<shell_content> shell_section::add(content_page& page)
    {
        std::shared_ptr<shell_content> content = shell_content::adopt(page);
        add(content);
        return content;
    }

    void shell_section::remove(const shell_content& content)
    {
        const auto it = std::ranges::find_if(items_, [&content](const std::shared_ptr<shell_content>& candidate) {
            return candidate.get() == &content;
        });
        if (it == items_.end())
        {
            return;
        }
        // Move the owning ref out before erasing the (now-emptied) slot, so the content stays alive
        // through the current-item transition without an extra copy.
        const std::shared_ptr<shell_content> removed = std::move(*it);
        items_.erase(it);
        detach_logical_child(*removed);
        if (wrapper_source_ == removed.get())
        {
            wrapper_sync_token_.reset();
            wrapper_source_ = nullptr;
        }
        // OnVisibleChildRemoved: re-select (or clear) the current item, then refresh the page.
        if (current_item_ == removed.get())
        {
            const std::vector<shell_content*> visible = visible_items();
            set_current_item(visible.empty() ? nullptr : visible.front());
        }
        update_displayed_page();
        send_structure_changed();
    }

    std::vector<shell_content*> shell_section::visible_items() const
    {
        std::vector<shell_content*> visible;
        visible.reserve(items_.size());
        for (const std::shared_ptr<shell_content>& content : items_)
        {
            if (content->is_visible())
            {
                visible.push_back(content.get());
            }
        }
        return visible;
    }

    void shell_section::set_current_item(shell_content* value)
    {
        if (current_item_ == value)
        {
            return;
        }
        shell_content* old_item = current_item_;
        current_item_ = value;

        // OnCurrentItemChanged: the old content disappears first; a null new value stops there.
        if (old_item != nullptr)
        {
            old_item->send_disappearing();
        }
        if (value == nullptr)
        {
            on_property_changed("current_item");
            return;
        }

        presented_page_appearing();

        if (shell* host = containing_shell(); host != nullptr && is_visible_section())
        {
            host->update_current_state(shell_navigation_source::shell_content_changed);
        }
        send_structure_changed();
        update_displayed_page();
        on_property_changed("current_item");
    }

    content_page* shell_section::presented_page() const
    {
        if (nav_stack_.size() > 1)
        {
            return nav_stack_.back();
        }
        return current_item_ != nullptr ? current_item_->page() : nullptr;
    }

    void shell_section::update_displayed_page()
    {
        content_page* previous = displayed_page_;
        if (nav_stack_.size() > 1)
        {
            displayed_page_ = nav_stack_.back();
        }
        else
        {
            displayed_page_ = current_item_ != nullptr ? current_item_->page() : nullptr;
        }

        if (previous != displayed_page_)
        {
            if (previous != nullptr)
            {
                previous->send_disappearing();
            }
            presented_page_appearing();
        }
    }

    bool shell_section::is_visible_section() const
    {
        const shell* host = containing_shell();
        return host != nullptr && host->current_item() != nullptr && host->current_item()->current_item() == this;
    }

    std::shared_ptr<shell_section> shell_section::create_from_shell_content(std::shared_ptr<shell_content> content)
    {
        // An already-parented content returns (and selects into) its existing section.
        if (auto* current = dynamic_cast<shell_section*>(content->logical_parent()))
        {
            const auto it =
                std::ranges::find_if(current->items_, [&content](const std::shared_ptr<shell_content>& candidate) {
                    return candidate == content;
                });
            if (it != current->items_.end())
            {
                current->set_current_item(content.get());
            }
            if (auto owner = current->weak_from_this().lock())
            {
                return std::static_pointer_cast<shell_section>(std::move(owner));
            }
        }

        auto section = std::make_shared<shell_section>();
        section->set_route(routing::generate_implicit_route(content->route()));

        shell_content* source = content.get();
        section->add(std::move(content));

        // The C# Title/Icon/FlyoutIcon (OneWay) bindings, as a live sync (§8: the token member is
        // declared after items_, so it disconnects before the owned source dies).
        section->set_title(std::string{source->title()});
        section->set_icon(source->icon());
        section->set_flyout_icon(source->flyout_icon());
        shell_section* self = section.get();
        section->wrapper_source_ = source;
        section->wrapper_sync_token_ =
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
        return section;
    }

    // ---- the stack mutators (C# OnPushAsync / OnPopAsync / OnPopToRootAsync / OnRemovePage /
    // OnInsertPageBefore) ----

    void shell_section::on_push(content_page& page, bool animated)
    {
        (void)animated; // threaded to the handler's transition in C#; no handler this unit
        std::vector<content_page*> proposed = nav_stack_;
        proposed.push_back(&page);
        if (!propose_navigation(shell_navigation_source::push, &proposed))
        {
            return;
        }

        presented_page_disappearing();
        nav_stack_.push_back(&page);
        presented_page_appearing();
        add_page(page);
    }

    content_page* shell_section::on_pop(bool animated)
    {
        (void)animated;
        if (nav_stack_.size() <= 1)
        {
            throw std::runtime_error{"Can't pop last page off stack"}; // C# InvalidOperationException
        }

        std::vector<content_page*> proposed = nav_stack_;
        proposed.pop_back();
        if (!propose_navigation(shell_navigation_source::pop, &proposed))
        {
            return nullptr;
        }

        content_page* page = nav_stack_.back();
        presented_page_disappearing();
        nav_stack_.pop_back();
        presented_page_appearing();
        remove_page(*page);
        return page;
    }

    void shell_section::on_pop_to_root(bool animated)
    {
        (void)animated;
        if (nav_stack_.size() <= 1)
        {
            return;
        }
        if (!propose_navigation(shell_navigation_source::pop_to_root, nullptr))
        {
            return;
        }

        presented_page_disappearing();
        std::vector<content_page*> old_stack = std::move(nav_stack_);
        nav_stack_ = {nullptr};
        presented_page_appearing();

        for (std::size_t i = 1; i < old_stack.size(); ++i)
        {
            // SendDisappearing only for intermediate pages; the top page's disappearing already fired.
            if (i < old_stack.size() - 1)
            {
                old_stack[i]->send_disappearing();
            }
            remove_page(*old_stack[i]);
        }
    }

    void shell_section::on_remove_page(content_page& page)
    {
        if (std::ranges::find(nav_stack_, &page) == nav_stack_.end())
        {
            return;
        }

        const bool current_page = presented_page() == &page;
        std::vector<content_page*> proposed = nav_stack_;
        std::erase(proposed, &page);
        const bool allow = !current_page || propose_navigation(shell_navigation_source::remove, &proposed);
        if (!allow)
        {
            return;
        }

        if (current_page)
        {
            presented_page_disappearing();
        }
        std::erase(nav_stack_, &page);
        if (current_page)
        {
            presented_page_appearing();
        }
        // §8: a route-created page is owned by this section (owned_pages_). Disconnect the handler
        // BEFORE remove_page drops the section's retention — and keep the page alive across both with a
        // local strong ref (mirrors C#'s GC, where the page survives RemovePage as long as a caller
        // holds it; the C# order RemovePage→DisconnectHandlers is GC-safe but a UAF without retention).
        const std::shared_ptr<content_page> keep_alive = page_owner(page);
        page.set_handler(nullptr); // C# page?.DisconnectHandlers()
        remove_page(page);
    }

    void shell_section::on_insert_page_before(content_page& page, content_page& before)
    {
        const auto it = std::ranges::find(nav_stack_, &before);
        if (it == nav_stack_.end())
        {
            throw std::invalid_argument{"Page not found in nav stack"};
        }
        const std::ptrdiff_t index = it - nav_stack_.begin();

        std::vector<content_page*> proposed = nav_stack_;
        proposed.insert(proposed.begin() + index, &page);
        if (!propose_navigation(shell_navigation_source::insert, &proposed))
        {
            return;
        }

        nav_stack_.insert(nav_stack_.begin() + index, &page);
        add_page(page);
    }

    // ---- GoToAsync (ShellSection.GoToAsync + PrepareCurrentStackForBeingReplaced, no modal) ----

    void shell_section::go_to(const shell_navigation_request& request, shell_route_parameters& query_data,
                              std::optional<bool> animate, bool is_relative_popping)
    {
        const std::vector<std::string>& global_routes = request.definition().global_routes();
        if (global_routes.empty())
        {
            if (nav_stack_.size() == 2)
            {
                (void)on_pop(animate.value_or(true));
            }
            else
            {
                on_pop_to_root(animate.value_or(true));
            }
            return;
        }

        prepare_current_stack_for_being_replaced(request, query_data, animate, global_routes, is_relative_popping);

        std::vector<std::shared_ptr<content_page>> non_modal_page_stack;
        const std::vector<content_page*> current_nav_stack = nav_stack_; // flattened == nav stack (no modal)

        std::size_t where_to_start_navigation = 0;
        if (request.stack_request() == shell_uri_handler::stack_request_kind::replace_it)
        {
            where_to_start_navigation = current_nav_stack.size() - 1;
        }

        for (std::size_t i = where_to_start_navigation; i < global_routes.size(); ++i)
        {
            const bool is_last = i == global_routes.size() - 1;
            std::shared_ptr<content_page> content = get_or_create_from_route(global_routes[i], query_data, is_last,
                                                                             /*is_popping=*/false);
            if (content == nullptr)
            {
                break;
            }
            non_modal_page_stack.push_back(std::move(content));
        }

        push_stack_of_pages(non_modal_page_stack, animate);
    }

    void shell_section::prepare_current_stack_for_being_replaced(const shell_navigation_request& request,
                                                                 shell_route_parameters& query_data,
                                                                 std::optional<bool> animate,
                                                                 const std::vector<std::string>& global_routes,
                                                                 bool is_relative_popping)
    {
        if (request.stack_request() != shell_uri_handler::stack_request_kind::replace_it)
        {
            return;
        }

        remove_excess_paths_within_the_route(global_routes);

        // Insert / push the pages that will become visible (the C# no-modal branch).
        std::vector<std::shared_ptr<content_page>> pages_to_insert;
        for (std::size_t i = 0; i < global_routes.size(); ++i)
        {
            const bool is_last = i == global_routes.size() - 1;
            const std::size_t nav_index = i + 1;
            // Routes match so don't do anything.
            if (nav_index < nav_stack_.size() && routing::get_route(*nav_stack_[nav_index]) == global_routes[i])
            {
                continue;
            }

            std::shared_ptr<content_page> page =
                get_or_create_from_route(global_routes[i], query_data, is_last, /*is_popping=*/false);
            if (page == nullptr)
            {
                continue;
            }
            if (!is_last && nav_index < nav_stack_.size())
            {
                retain_page(page);
                on_insert_page_before(*page, *nav_stack_[nav_index]);
            }
            else
            {
                pages_to_insert.push_back(std::move(page));
            }
        }

        push_stack_of_pages(pages_to_insert, animate);
        remove_excess_paths_within_the_route(global_routes);

        // Pop the stack down to where it no longer matches.
        for (std::size_t i = 0; i < global_routes.size(); ++i)
        {
            const bool is_last = i == global_routes.size() - 1;
            const std::string& route = global_routes[i];

            std::vector<content_page*> nav_stack = nav_stack_;
            // if the navStack count is one that means there is nothing pushed
            if (nav_stack.size() == 1)
            {
                break;
            }

            content_page* nav_page = nav_stack.size() > i + 1 ? nav_stack[i + 1] : nullptr;
            if (nav_page == nullptr)
            {
                continue;
            }

            std::size_t pop_count = i + 1;
            if (routing::get_route(*nav_page) == route)
            {
                // if the routes match and this is the last loop, pop everything after this route
                pop_count = i + 2;
                shell_navigation_manager::apply_query_attributes(*nav_page, query_data, is_last, is_relative_popping);
                // If we're not on the last loop of the stack then continue
                if (!is_last)
                {
                    continue;
                }
            }

            while (nav_stack_.size() > pop_count)
            {
                // Remove middle pages before popping the visible page so the transition is seamless.
                if (nav_stack_.size() - pop_count == 1)
                {
                    (void)on_pop(animate.value_or(true));
                }
                else
                {
                    on_remove_page(*nav_stack_[nav_stack_.size() - 2]);
                }
            }
            break;
        }
    }

    void shell_section::remove_excess_paths_within_the_route(const std::vector<std::string>& global_routes)
    {
        // locate middle routes that were removed: //route/page1/page2 => //route/page2
        for (std::size_t i = 0; i < global_routes.size(); ++i)
        {
            std::ptrdiff_t found_match_at = -1;
            for (std::size_t j = 1; j < nav_stack_.size(); ++j)
            {
                if (routing::get_route(*nav_stack_[j]) == global_routes[i])
                {
                    found_match_at = static_cast<std::ptrdiff_t>(j);
                    break;
                }
            }

            // If we found a matching route then remove all the middle pages.
            for (std::ptrdiff_t j = found_match_at - 1; j >= static_cast<std::ptrdiff_t>(i) + 1; --j)
            {
                on_remove_page(*nav_stack_[static_cast<std::size_t>(j)]);
            }
        }
    }

    std::shared_ptr<content_page> shell_section::get_or_create_from_route(const std::string& route,
                                                                          shell_route_parameters& query_data,
                                                                          bool is_last, bool is_popping)
    {
        std::shared_ptr<content_page> content = routing::get_or_create_content(route);
        if (content == nullptr)
        {
            // C# logs "Failed to Create Content For: {route}" and continues with null.
            return nullptr;
        }
        shell_navigation_manager::apply_query_attributes(*content, query_data, is_last, is_popping);
        return content;
    }

    void shell_section::push_stack_of_pages(const std::vector<std::shared_ptr<content_page>>& pages,
                                            std::optional<bool> animate)
    {
        for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(pages.size()) - 1; i >= 0; --i)
        {
            const auto& page = pages[static_cast<std::size_t>(i)];
            retain_page(page);
            const bool is_last = i == static_cast<std::ptrdiff_t>(pages.size()) - 1;
            if (is_last)
            {
                on_push(*page, animate.value_or(true));
            }
            else
            {
                on_insert_page_before(*page, *pages[static_cast<std::size_t>(i) + 1]);
            }
            // A proposal-rejected push leaves the page off the stack — drop the keep-alive again.
            if (std::ranges::find(nav_stack_, page.get()) == nav_stack_.end())
            {
                owned_pages_.erase(page.get());
            }
        }
    }

    void shell_section::retain_page(const std::shared_ptr<content_page>& page)
    {
        owned_pages_.insert_or_assign(page.get(), page);
    }

    std::shared_ptr<content_page> shell_section::page_owner(content_page& page)
    {
        const auto it = owned_pages_.find(&page);
        return it != owned_pages_.end() ? it->second : nullptr;
    }

    void shell_section::add_page(content_page& page)
    {
        attach_logical_child(page);
    }

    void shell_section::remove_page(content_page& page)
    {
        detach_logical_child(page);
        owned_pages_.erase(&page); // a route-created page's life ends with its stack membership
    }

    // ---- lifecycle ----

    void shell_section::presented_page_disappearing()
    {
        if (current_item_ != nullptr)
        {
            current_item_->send_disappearing();
        }
        content_page* presented = presented_page();
        if (presented != nullptr && is_visible_section())
        {
            presented->send_disappearing();
        }
    }

    void shell_section::presented_page_appearing()
    {
        if (!is_visible_section())
        {
            return;
        }
        if (nav_stack_.size() == 1 && current_item_ != nullptr)
        {
            current_item_->send_appearing();
        }
        if (content_page* presented = presented_page())
        {
            // DEVIATION (documented): C# defers this until the page is parented (the ParentSet wait)
            // when Parent is still null; the port appears the page immediately — on_push attaches it
            // as a logical child in the same synchronous mutation, so only the relative order of the
            // appearing event vs. the parent attach differs (no self-owning event subscription, §8).
            presented->send_appearing();
        }
    }

    void shell_section::send_appearing()
    {
        base_shell_item::send_appearing();
        presented_page_appearing();
    }

    void shell_section::send_disappearing()
    {
        base_shell_item::send_disappearing();
        presented_page_disappearing();
    }

    shell* shell_section::containing_shell() const
    {
        const shell_item* item = parent_item();
        return item != nullptr ? dynamic_cast<shell*>(item->logical_parent()) : nullptr;
    }

    shell_item* shell_section::parent_item() const
    {
        return dynamic_cast<shell_item*>(logical_parent());
    }

    void shell_section::send_structure_changed() const
    {
        if (shell* host = containing_shell())
        {
            if (is_visible_section())
            {
                host->send_structure_changed();
            }
            host->send_flyout_items_changed();
        }
    }

    bool shell_section::propose_navigation(shell_navigation_source source, const std::vector<content_page*>* stack)
    {
        shell* host = containing_shell();
        if (host == nullptr)
        {
            return true; // no shell yet — nothing to propose against
        }
        return host->propose_navigation(source, parent_item(), this, current_item_, stack, true);
    }

    void shell_section::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const std::shared_ptr<shell_content>& content : items_)
        {
            visit(*content);
        }
        for (content_page* page : nav_stack_)
        {
            if (page != nullptr)
            {
                visit(*page);
            }
        }
    }
} // namespace maui::controls
