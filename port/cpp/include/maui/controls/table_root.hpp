#pragma once
// maui::controls::table_root  <=  Microsoft.Maui.Controls.TableRoot
//   (the sealed TableSectionBase<TableSection> at the top of a table_view)
//
// The root of a table_view: an ordered list of co-owned table_sections. Ported from
// src/Controls/src/Core/TableView/TableRoot.cs (+ the TableSectionBase<TableSection> half of
// TableSection.cs). It:
//   - owns the sections (add/insert/remove/clear) with a collection_changed notification,
//   - re-broadcasts each child section's cell collection_changed as section_collection_changed (so the
//     table_view can re-parent the newly-added cells) — TableRoot.ChildCollectionChanged,
//   - bubbles a child section's Title change up as its own property_changed("title") — TableRoot
//     .ChildPropertyChanged (the table_view re-runs its model on a section-title change),
//   - propagates its binding context down to every section.

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_section_base.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class table_root : public table_section_base
    {
    public:
        table_root();
        explicit table_root(std::string title);
        ~table_root() override;
        table_root(const table_root&) = delete;
        table_root(table_root&&) = delete;
        table_root& operator=(const table_root&) = delete;
        table_root& operator=(table_root&&) = delete;

        // A child section's cells changed (TableRoot.SectionCollectionChanged): the section + its args.
        struct section_change
        {
            table_section* section = nullptr;
            collection_changed_args<std::shared_ptr<cell>> args;
        };

        // INotifyCollectionChanged for the sections themselves (TableRoot.CollectionChanged).
        maui::core::event<collection_changed_args<std::shared_ptr<table_section>>> collection_changed;
        // A child section's cell list changed (TableRoot.SectionCollectionChanged).
        maui::core::event<section_change> section_collection_changed;

        [[nodiscard]] std::size_t count() const
        {
            return sections_.size();
        }
        [[nodiscard]] const std::shared_ptr<table_section>& at(std::size_t index) const
        {
            return sections_.at(index);
        }
        [[nodiscard]] const std::vector<std::shared_ptr<table_section>>& sections() const
        {
            return sections_;
        }

        void add(std::shared_ptr<table_section> item);
        void insert(std::size_t index, std::shared_ptr<table_section> item);
        bool remove(const std::shared_ptr<table_section>& item);
        void clear();

    protected:
        // TableSectionBase<T>.OnBindingContextChanged: push the new context into every section.
        void on_binding_context_changed() override;

    private:
        // The per-section subscriptions (collection_changed + property_changed), held beside the section
        // pointer so a single section can be unhooked. Move-only (scoped_connection is move-only); kept so
        // they tear down before the publishers (§8). hook_section subscribes; the destructor / clear() /
        // remove() drop the matching entry first (subscriber before publisher).
        struct section_subscription
        {
            table_section* section = nullptr;
            maui::core::scoped_connection collection_changed_token;
            maui::core::scoped_connection title_changed_token;
        };

        // TableRoot.SetupEvents: re-broadcast the section's collection_changed + bubble its title change.
        void hook_section(const std::shared_ptr<table_section>& section);
        void unhook_section(const table_section* section);

        std::vector<std::shared_ptr<table_section>> sections_; // TableSectionBase<T>._children (co-owned)
        std::vector<section_subscription> subscriptions_;      // after sections_ — torn down first (§8)
    };
} // namespace maui::controls
