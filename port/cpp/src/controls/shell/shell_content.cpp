// maui::controls::shell_content — out-of-line bodies. See shell_content.hpp.

#include "maui/controls/shell/shell_content.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/i_query_attributable.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    namespace
    {
        // The reflection-free slice of C#'s static ApplyQueryAttributes(content, query, old): deliver
        // the dictionary to the page when it implements i_query_attributable (the only channel).
        void apply_attributes_to_page(content_page& page, const shell_route_parameters& query)
        {
            if (auto* attributable = dynamic_cast<i_query_attributable*>(&page))
            {
                attributable->apply_query_attributes(query);
            }
        }
    } // namespace

    shell_content::~shell_content() = default;

    void shell_content::set_content(content_page* value)
    {
        // OnContentChanged: without a template the page is displayed eagerly.
        if (content_template_ == nullptr)
        {
            set_content_cache(value);
        }
        content_ = value;
        if (const shell_section* section = parent_section())
        {
            if (auto* item = dynamic_cast<shell_item*>(section->logical_parent()))
            {
                item->send_structure_changed();
            }
        }
    }

    content_page* shell_content::get_or_create_content()
    {
        content_page* result = nullptr;
        if (content_template_ == nullptr)
        {
            result = content_;
        }
        else
        {
            if (content_cache_ != nullptr)
            {
                result = content_cache_;
            }
            else
            {
                owned_page_ = std::dynamic_pointer_cast<content_page>(content_template_->create_content());
                result = owned_page_.get();
            }
            // The delayed QueryAttributes flow to the freshly-created page (C# SetValue on result).
            if (result != nullptr && query_attributes_set_)
            {
                apply_attributes_to_page(*result, query_attributes_);
            }
            set_content_cache(result);
        }

        if (result == nullptr)
        {
            throw std::runtime_error{"No Content found for shell_content, Title:" + std::string{title()} + ", Route " +
                                     route()};
        }
        return result;
    }

    std::shared_ptr<shell_content> shell_content::adopt(content_page& page)
    {
        // An already-wrapped page returns its existing wrapper (C# `return (ShellContent)page.Parent`).
        if (auto* existing = dynamic_cast<shell_content*>(page.logical_parent()))
        {
            if (auto owner = existing->weak_from_this().lock())
            {
                return std::static_pointer_cast<shell_content>(std::move(owner));
            }
        }

        auto wrapper = std::make_shared<shell_content>();
        wrapper->set_route(routing::generate_implicit_route(routing::get_route(page)));
        wrapper->set_content(&page);
        // The C# Title binding (source: page), as a live sync. The page must outlive the wrapper —
        // the token disconnects in the wrapper's destruction (§8). The Icon/FlyoutIcon bindings have
        // no source on the port's content_page (no Icon property) and are dropped.
        wrapper->set_title(std::string{page.title()});
        shell_content* self = wrapper.get();
        const content_page* source = &page;
        wrapper->adopted_title_token_ =
            maui::core::connect_scoped(page.property_changed, [self, source](std::string_view name) {
                if (name == "title")
                {
                    self->set_title(std::string{source->title()});
                }
            });
        return wrapper;
    }

    void shell_content::apply_query_attributes(const shell_route_parameters& query)
    {
        // An empty query only matters when something was previously propagated (the C# early-out).
        if (query.empty() && !query_attributes_set_)
        {
            return;
        }
        query_attributes_ = query;
        query_attributes_set_ = true;
        if (content_cache_ != nullptr)
        {
            apply_attributes_to_page(*content_cache_, query_attributes_);
        }
    }

    bool shell_content::is_visible_content() const
    {
        const shell_section* section = parent_section();
        return section != nullptr && section->is_visible_section() && section->current_item() == this;
    }

    void shell_content::send_appearing()
    {
        // Only fire Appearing once a page exists on this content (the C# content==null early-out).
        content_page* page = content_cache_ != nullptr ? content_cache_ : content_;
        if (page == nullptr)
        {
            return;
        }
        base_shell_item::send_appearing();
        if (is_visible_content())
        {
            page->send_appearing();
        }
    }

    void shell_content::send_disappearing()
    {
        base_shell_item::send_disappearing();
        if (content_page* page = content_cache_ != nullptr ? content_cache_ : content_)
        {
            page->send_disappearing();
        }
    }

    void shell_content::on_appearing(std::function<void()> action)
    {
        if (has_appeared())
        {
            action();
            return;
        }
        // BaseShellItem.OnAppearing's Navigation.NavigationStack branch: a pushed page is visible even
        // though the content itself has disappeared (PresentedPageDisappearing sends the content away on
        // every push). Delegate to the TOP nav-stack page — Page.OnAppearing(Action) runs the action now
        // if that page has appeared, else queues it on the page's Appearing (a self-removing one-shot).
        if (const shell_section* section = parent_section())
        {
            const std::vector<content_page*>& stack = section->stack();
            if (stack.size() > 1)
            {
                content_page* top = stack.back();
                if (top != nullptr)
                {
                    if (top->has_appeared())
                    {
                        action();
                        return;
                    }
                    auto token = std::make_shared<maui::core::connection_token>(0);
                    *token = top->appearing.connect([top, token, action = std::move(action)]() {
                        top->appearing.disconnect(*token);
                        action();
                    });
                    return;
                }
            }
        }
        // The content has not appeared and nothing is on the stack: run when it next appears (the base
        // behaviour — queue on this content's own appearing).
        base_shell_item::on_appearing(std::move(action));
    }

    void shell_content::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        if (content_cache_ != nullptr)
        {
            visit(*content_cache_);
        }
    }

    void shell_content::set_content_cache(content_page* value)
    {
        if (content_cache_ == value)
        {
            return;
        }
        content_page* old_cache = content_cache_;
        content_cache_ = value;
        if (old_cache != nullptr)
        {
            detach_logical_child(*old_cache);
        }
        if (value != nullptr && value->logical_parent() != this)
        {
            attach_logical_child(*value);
        }
        if (old_cache != nullptr && old_cache == owned_page_.get() && value != old_cache)
        {
            owned_page_.reset(); // the replaced template page's ownership ends with the cache slot
        }
        if (shell_section* section = parent_section())
        {
            section->update_displayed_page();
        }
    }

    shell_section* shell_content::parent_section() const
    {
        return dynamic_cast<shell_section*>(logical_parent());
    }
} // namespace maui::controls
