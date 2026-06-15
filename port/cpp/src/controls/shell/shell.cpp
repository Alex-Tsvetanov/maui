// maui::controls::shell — out-of-line bodies. See shell.hpp.

#include "maui/controls/shell/shell.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/flyout_header_behavior.hpp"
#include "maui/controls/shell/route_request_builder.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell_appearance.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_navigated_event_args.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    namespace
    {
        // The process-wide attached-property side map for Shell.SearchHandler, keyed by the TARGET
        // bindable_object pointer (the port has no central attached-property store — the routing
        // element-route side map precedent, routing.cpp). Unlike routing (whose elements call
        // remove_route in their dtor), the attached target here can be ANY bindable_object (typically a
        // content_page, a layer BELOW shell that cannot call back into shell), so each entry also captures
        // the target's weak liveness token (bindable_object::weak_token, PROFILE §8); a stale/dead entry
        // (token expired, so the raw pointer may have been recycled) is pruned on access — self-cleaning,
        // no dtor hook or layer-inverting dependency. Owned by a function-local static so its lifetime
        // spans every translation unit.
        struct search_handler_entry
        {
            std::weak_ptr<void> liveness;
            std::shared_ptr<search_handler> handler;
        };

        std::unordered_map<const maui::core::bindable_object*, search_handler_entry>& search_handler_map()
        {
            static std::unordered_map<const maui::core::bindable_object*, search_handler_entry> map;
            return map;
        }

        // The process-wide side map for the Shell.* appearance ATTACHED properties (the C# attached-property
        // store the port lacks centrally), keyed by the TARGET element pointer. Same self-cleaning weak-token
        // hygiene as search_handler_map: each entry captures the element's weak liveness token so a stale
        // entry (the element died, the raw pointer possibly recycled) is pruned on access — no dtor hook or
        // layer-inverting dependency on shell. Owned by a function-local static (spans every TU).
        struct appearance_entry
        {
            std::weak_ptr<void> liveness;
            shell::appearance_values values;
        };

        std::unordered_map<const element*, appearance_entry>& appearance_map()
        {
            static std::unordered_map<const element*, appearance_entry> map;
            return map;
        }

        // The set-values bag for `target`, pruning a dead entry. Returns nullptr when nothing is set / stale.
        shell::appearance_values* find_appearance_values(const element& target)
        {
            auto& map = appearance_map();
            const auto it = map.find(&target);
            if (it == map.end())
            {
                return nullptr;
            }
            if (it->second.liveness.expired())
            {
                map.erase(it);
                return nullptr;
            }
            return &it->second.values;
        }

        // The mutable bag for `target`, creating it (capturing the liveness token) on first write.
        shell::appearance_values& appearance_values_for_write(const element& target)
        {
            auto& map = appearance_map();
            appearance_entry& entry = map[&target];
            if (entry.liveness.expired())
            {
                // Fresh or RECYCLED slot: the raw pointer may have belonged to a now-dead element (its token
                // expired). Start a clean bag for THIS element and (re)capture its live token so it
                // self-prunes later — never inherit the recycled predecessor's values.
                entry.values = shell::appearance_values{};
                entry.liveness = target.weak_token();
            }
            return entry.values;
        }
    } // namespace

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

    const maui::core::bindable_property<flyout_header_behavior>& shell::flyout_header_behavior_property()
    {
        // C# FlyoutHeaderBehaviorProperty: default FlyoutHeaderBehavior.Default, BindingMode.OneTime.
        static const maui::core::bindable_property<flyout_header_behavior> descriptor{
            "flyout_header_behavior", flyout_header_behavior::default_behavior};
        return descriptor;
    }

    // ---- flyout header / footer (Shell.FlyoutHeader/FlyoutFooter + their *Template props) ----

    namespace
    {
        // ShellTemplatedViewManager resolution: a non-null DataTemplate wins (CreateContent() cast to a
        // View), else the raw header/footer view. A template that builds a non-View content resolves to
        // null (the C# `(View)Template.CreateContent(...)` / `as View` cast). Returns the resolved View
        // (null when nothing resolves).
        std::shared_ptr<maui::core::i_view> resolve_templated_view(const std::shared_ptr<data_template>& tmpl,
                                                                   const std::shared_ptr<maui::core::i_view>& raw)
        {
            if (tmpl != nullptr)
            {
                const std::shared_ptr<maui::core::bindable_object> content = tmpl->create_content();
                return std::dynamic_pointer_cast<maui::core::i_view>(content);
            }
            return raw;
        }
    } // namespace

    void shell::set_flyout_header(std::shared_ptr<maui::core::i_view> value)
    {
        if (flyout_header_ == value)
        {
            return;
        }
        flyout_header_ = std::move(value);
        resolve_flyout_header_view();
    }

    void shell::set_flyout_footer(std::shared_ptr<maui::core::i_view> value)
    {
        if (flyout_footer_ == value)
        {
            return;
        }
        flyout_footer_ = std::move(value);
        resolve_flyout_footer_view();
    }

    void shell::set_flyout_header_template(std::shared_ptr<data_template> value)
    {
        if (flyout_header_template_ == value)
        {
            return;
        }
        flyout_header_template_ = std::move(value);
        resolve_flyout_header_view();
    }

    void shell::set_flyout_footer_template(std::shared_ptr<data_template> value)
    {
        if (flyout_footer_template_ == value)
        {
            return;
        }
        flyout_footer_template_ = std::move(value);
        resolve_flyout_footer_view();
    }

    void shell::resolve_flyout_header_view()
    {
        std::shared_ptr<maui::core::i_view> resolved = resolve_templated_view(flyout_header_template_, flyout_header_);
        if (resolved == flyout_header_view_)
        {
            return;
        }
        // ShellTemplatedViewManager.SetView's RemoveLogicalChild / AddLogicalChild — flow this shell's
        // binding context into the header chrome view (and drop it from the previous one).
        if (auto* old_child = dynamic_cast<element*>(flyout_header_view_.get()))
        {
            element::detach_logical_child(*old_child);
        }
        flyout_header_view_ = std::move(resolved);
        if (auto* new_child = dynamic_cast<element*>(flyout_header_view_.get()))
        {
            this->attach_logical_child(*new_child);
        }
        on_property_changed("flyout_header"); // the chrome re-materializes the header
    }

    void shell::resolve_flyout_footer_view()
    {
        std::shared_ptr<maui::core::i_view> resolved = resolve_templated_view(flyout_footer_template_, flyout_footer_);
        if (resolved == flyout_footer_view_)
        {
            return;
        }
        if (auto* old_child = dynamic_cast<element*>(flyout_footer_view_.get()))
        {
            element::detach_logical_child(*old_child);
        }
        flyout_footer_view_ = std::move(resolved);
        if (auto* new_child = dynamic_cast<element*>(flyout_footer_view_.get()))
        {
            this->attach_logical_child(*new_child);
        }
        on_property_changed("flyout_footer"); // the chrome re-materializes the footer
    }

    // ---- Shell.SearchHandler (attached property) ----

    void shell::set_search_handler(maui::core::bindable_object& target, std::shared_ptr<search_handler> handler)
    {
        auto& map = search_handler_map();
        // C# OnSearchHandlerPropertyChanged: detach the old handler's inherited binding context, then flow
        // the target's context into the new handler (SetInheritedBindingContext).
        if (const auto it = map.find(&target); it != map.end() && it->second.handler)
        {
            it->second.handler->set_inherited_binding_context({});
        }
        if (handler == nullptr)
        {
            map.erase(&target);
            return;
        }
        handler->set_inherited_binding_context(target.raw_binding_context());
        map.insert_or_assign(&target,
                             search_handler_entry{.liveness = target.weak_token(), .handler = std::move(handler)});
    }

    search_handler* shell::get_search_handler(const maui::core::bindable_object& target)
    {
        return get_search_handler_shared(target).get();
    }

    std::shared_ptr<search_handler> shell::get_search_handler_shared(const maui::core::bindable_object& target)
    {
        auto& map = search_handler_map();
        const auto it = map.find(&target);
        if (it == map.end())
        {
            return nullptr;
        }
        // Prune a stale entry: the target died and the raw pointer may have been recycled into a new,
        // unrelated object (the weak token would belong to the dead original).
        if (it->second.liveness.expired())
        {
            map.erase(it);
            return nullptr;
        }
        return it->second.handler;
    }

    void shell::remove_search_handler(const maui::core::bindable_object& target)
    {
        search_handler_map().erase(&target);
    }

    // ---- Shell appearance attached properties + the resolution walk ----

    bool shell::appearance_values::any_set() const
    {
        return background_color || disabled_color || foreground_color || tab_bar_background_color ||
               tab_bar_disabled_color || tab_bar_foreground_color || tab_bar_title_color || tab_bar_unselected_color ||
               title_color || unselected_color || flyout_width || flyout_height;
    }

    void shell::set_background_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).background_color = value;
    }
    void shell::set_disabled_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).disabled_color = value;
    }
    void shell::set_foreground_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).foreground_color = value;
    }
    void shell::set_tab_bar_background_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).tab_bar_background_color = value;
    }
    void shell::set_tab_bar_disabled_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).tab_bar_disabled_color = value;
    }
    void shell::set_tab_bar_foreground_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).tab_bar_foreground_color = value;
    }
    void shell::set_tab_bar_title_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).tab_bar_title_color = value;
    }
    void shell::set_tab_bar_unselected_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).tab_bar_unselected_color = value;
    }
    void shell::set_title_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).title_color = value;
    }
    void shell::set_unselected_color(element& target, maui::graphics::color value)
    {
        appearance_values_for_write(target).unselected_color = value;
    }
    void shell::set_flyout_width(element& target, double value)
    {
        appearance_values_for_write(target).flyout_width = value;
    }
    void shell::set_flyout_height(element& target, double value)
    {
        appearance_values_for_write(target).flyout_height = value;
    }

    std::optional<maui::graphics::color> shell::get_background_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->background_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_disabled_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->disabled_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_foreground_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->foreground_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_tab_bar_background_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->tab_bar_background_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_tab_bar_disabled_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->tab_bar_disabled_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_tab_bar_foreground_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->tab_bar_foreground_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_tab_bar_title_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->tab_bar_title_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_tab_bar_unselected_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->tab_bar_unselected_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_title_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->title_color : std::nullopt;
    }
    std::optional<maui::graphics::color> shell::get_unselected_color(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->unselected_color : std::nullopt;
    }
    std::optional<double> shell::get_flyout_width(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->flyout_width : std::nullopt;
    }
    std::optional<double> shell::get_flyout_height(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? v->flyout_height : std::nullopt;
    }

    shell::appearance_values shell::values_of(const element& target)
    {
        const appearance_values* const v = find_appearance_values(target);
        return v != nullptr ? *v : appearance_values{};
    }

    void shell::remove_appearance_values(const element& target)
    {
        appearance_map().erase(&target);
    }

    namespace
    {
        // Shell.WalkToPage: descend the selection chain to the leaf the appearance should resolve from.
        //   Shell        -> CurrentItem (shell_item)
        //   ShellItem    -> CurrentItem (shell_section)
        //   ShellSection -> PresentedPage ?? itself (the section's top page; the C# "PresentedPage ?? element")
        // Any other element (already a page / content) returns unchanged.
        element* walk_to_page(element* node)
        {
            if (auto* host = dynamic_cast<shell*>(node))
            {
                return host->current_item();
            }
            if (auto* item = dynamic_cast<shell_item*>(node))
            {
                return item->current_item();
            }
            if (auto* section = dynamic_cast<shell_section*>(node))
            {
                content_page* const page = section->presented_page();
                return page != nullptr ? static_cast<element*>(page) : static_cast<element*>(section);
            }
            return node;
        }
    } // namespace

    std::optional<shell_appearance> shell::get_appearance_for_pivot(element& pivot)
    {
        // The C# GetAppearanceForPivot algorithm:
        //  1) walk DOWN to the current page (walk_to_page),
        //  2) walk UP to the root shell, ingesting each level's set attached values (lowest wins per slot),
        //     plus — until a ShellContent has been seen — the current section's CurrentItem content (so the
        //     ROOT content is honored even with a page pushed on top, the C# "minor deviation" comment),
        //  3) return nullopt if nothing in the line set any value.
        element* node = walk_to_page(&pivot);

        bool found_shell_content = false;
        bool any_set = false;
        shell_appearance result;
        while (node != nullptr)
        {
            if (dynamic_cast<shell_content*>(node) != nullptr)
            {
                found_shell_content = true;
            }

            if (!found_shell_content)
            {
                if (const auto* section = dynamic_cast<const shell_section*>(node))
                {
                    if (const shell_content* const current = section->current_item(); current != nullptr)
                    {
                        if (result.ingest(*current))
                        {
                            any_set = true;
                        }
                    }
                }
            }

            if (result.ingest(*node))
            {
                any_set = true;
            }

            node = node->logical_parent();
        }

        if (any_set)
        {
            result.make_complete();
            return result;
        }
        return std::nullopt;
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
        // The resolved flyout header / footer views are logical children too (C# AddLogicalChild via
        // ShellTemplatedViewManager.SetView) so the base on_binding_context_changed re-flows this shell's
        // BindingContext into them on a later context change — matching Shell.OnBindingContextChanged's
        // SetInheritedBindingContext(FlyoutHeaderView/FlyoutFooterView, BindingContext).
        if (auto* header_child = dynamic_cast<element*>(flyout_header_view_.get()))
        {
            visit(*header_child);
        }
        if (auto* footer_child = dynamic_cast<element*>(flyout_footer_view_.get()))
        {
            visit(*footer_child);
        }
    }
} // namespace maui::controls
