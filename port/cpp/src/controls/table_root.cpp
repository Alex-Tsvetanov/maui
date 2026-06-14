// maui::controls::table_root — the section list: collection_changed, re-broadcast of section cell
// changes, title-change bubbling, and binding-context inheritance. See table_root.hpp; ported from
// src/Controls/src/Core/TableView/TableRoot.cs (+ the TableSectionBase<TableSection> half).

#include "maui/controls/table_root.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_section_base.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    table_root::table_root() = default;

    table_root::table_root(std::string title) : table_section_base(std::move(title))
    {
    }

    // §8: subscriptions_ is declared AFTER sections_, so it destructs FIRST — every section subscription
    // disconnects before its publisher section is destroyed. No explicit teardown needed.
    table_root::~table_root() = default;

    void table_root::add(std::shared_ptr<table_section> item)
    {
        insert(sections_.size(), std::move(item));
    }

    void table_root::insert(std::size_t index, std::shared_ptr<table_section> item)
    {
        if (item == nullptr || index > sections_.size())
        {
            return;
        }
        // Inherit the root's context (TableSectionBase<T>.OnChildrenChanged) + hook the section's events.
        item->set_inherited_binding_context(raw_binding_context());
        hook_section(item);
        sections_.insert(sections_.begin() + static_cast<std::ptrdiff_t>(index), item);
        collection_changed_args<std::shared_ptr<table_section>> args;
        args.action = collection_changed_action::add;
        args.new_items.push_back(std::move(item));
        args.new_starting_index = static_cast<std::ptrdiff_t>(index);
        collection_changed.raise(args);
    }

    bool table_root::remove(const std::shared_ptr<table_section>& item)
    {
        const auto found = std::ranges::find(sections_, item);
        if (found == sections_.end())
        {
            return false;
        }
        const auto index = static_cast<std::size_t>(std::distance(sections_.begin(), found));
        std::shared_ptr<table_section> removed = *found;
        unhook_section(removed.get()); // subscriber before publisher (§8)
        sections_.erase(found);
        collection_changed_args<std::shared_ptr<table_section>> args;
        args.action = collection_changed_action::remove;
        args.old_items.push_back(std::move(removed));
        args.old_starting_index = static_cast<std::ptrdiff_t>(index);
        collection_changed.raise(args);
        return true;
    }

    void table_root::clear()
    {
        subscriptions_.clear(); // drop every subscription before the sections go (§8)
        sections_.clear();
        collection_changed_args<std::shared_ptr<table_section>> args;
        args.action = collection_changed_action::reset;
        collection_changed.raise(args);
    }

    void table_root::on_binding_context_changed()
    {
        table_section_base::on_binding_context_changed();
        const auto& context = raw_binding_context();
        for (const auto& section : sections_)
        {
            if (section != nullptr)
            {
                section->set_inherited_binding_context(context);
            }
        }
    }

    void table_root::hook_section(const std::shared_ptr<table_section>& section)
    {
        section_subscription subscription;
        subscription.section = section.get();
        table_section* const raw = section.get();
        // Re-broadcast the section's cell collection_changed as our section_collection_changed.
        subscription.collection_changed_token = maui::core::connect_scoped(
            section->collection_changed, [this, raw](const collection_changed_args<std::shared_ptr<cell>>& args) {
                section_change change;
                change.section = raw;
                change.args = args;
                section_collection_changed.raise(change);
            });
        // Bubble the section's Title change up as our own property_changed("title")
        // (TableRoot.ChildPropertyChanged).
        subscription.title_changed_token =
            maui::core::connect_scoped(section->property_changed, [this](std::string_view name) {
                if (name == "title")
                {
                    on_property_changed("title");
                }
            });
        subscriptions_.push_back(std::move(subscription));
    }

    void table_root::unhook_section(const table_section* section)
    {
        const auto found = std::ranges::find_if(
            subscriptions_, [section](const section_subscription& s) { return s.section == section; });
        if (found != subscriptions_.end())
        {
            subscriptions_.erase(found);
        }
    }
} // namespace maui::controls
