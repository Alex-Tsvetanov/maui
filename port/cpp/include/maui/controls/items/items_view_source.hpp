#pragma once
// maui::controls::i_items_view_source  <=  Microsoft.Maui.Controls.Handlers.Items.IItemsViewSource
// (+ IObservableItemsViewSource), the iOS section/index-addressed flavor — the piece the platform
// items controllers consume (wave 3) and the headless virtualization simulator consumes today.
//
// index_path is the NSIndexPath stand-in ({section, item}; {-1,-1} = not found, like the C#
// NSIndexPath.Create(-1,-1) miss). source_update is the port's collapse of the C# sources' DIRECT
// UICollectionView drives (InsertItems/DeleteItems/ReloadItems/MoveItem/ReloadData and the section
// twins): each collection change is translated into ONE already-shaped op and raised on `updated`
// AFTER the source's own counts are adjusted — the consumer (simulator now, the native controllers
// in wave 3) applies it 1:1 to its viewport. The C# CollectionViewUpdating/Updated event pair
// collapses into this single post-state event (documented deviation: no pre-event consumer exists
// in the port).
//
// Concrete sources (items_source_factory.hpp <= ItemsSourceFactory): empty / list (snapshot) /
// observable / observable-grouped, internal like the C# classes.

#include <cstddef>
#include <cstdint>
#include <memory>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    // <= Foundation.NSIndexPath (reduced): a {section, item} address into a (possibly grouped) source.
    struct index_path
    {
        int section = -1;
        int item = -1;
        friend bool operator==(const index_path&, const index_path&) = default;
    };

    // The translated viewport op one collection change produces (header note).
    enum class source_update_kind : std::uint8_t
    {
        insert_items = 0,
        delete_items,
        reload_items,
        move_item,
        insert_sections,
        delete_sections,
        reload_sections,
        move_section,
        reload_data,
    };

    struct source_update
    {
        source_update_kind kind = source_update_kind::reload_data;
        // Item ops: the section + starting item index; section ops: `section` is the starting section
        // (index unused). reload_data carries neither.
        int section = 0;
        int index = -1;
        std::size_t count = 0; // items (or sections) affected
        // move_item: the destination path; move_section: destination section in `move_to.section`.
        index_path move_to{};
    };

    class i_items_view_source
    {
    public:
        virtual ~i_items_view_source() = default;

        [[nodiscard]] virtual int item_count() const = 0;
        [[nodiscard]] virtual int group_count() const = 0;
        [[nodiscard]] virtual int item_count_in_group(int group) const = 0;
        // The item at `path` (std::out_of_range on a bad section, like the C# ArgumentOutOfRange).
        [[nodiscard]] virtual boxed_item item(const index_path& path) const = 0;
        // The group key object at path.section (the group header/footer binding context); the null
        // item on a flat source (C# returns null).
        [[nodiscard]] virtual boxed_item group(const index_path& path) const = 0;
        // The section viewed as its own source (grouped only; null on a flat source).
        [[nodiscard]] virtual std::shared_ptr<i_items_view_source> group_items_view_source(
            const index_path& path) const = 0;
        // {-1,-1} when not found.
        [[nodiscard]] virtual index_path get_index_for_item(const boxed_item& item) const = 0;

        // The change fan-out (header note). Raised after this source's counts already reflect the change.
        maui::core::event<const source_update&> updated;

    protected:
        i_items_view_source() = default;
        i_items_view_source(const i_items_view_source&) = delete;
        i_items_view_source(i_items_view_source&&) = delete;
        i_items_view_source& operator=(const i_items_view_source&) = delete;
        i_items_view_source& operator=(i_items_view_source&&) = delete;
    };

    // <= IObservableItemsViewSource: the live flavors expose the ObserveChanges gate (the native
    // reorder path flips it off while it mutates the collection itself).
    class i_observable_items_view_source : public i_items_view_source
    {
    public:
        [[nodiscard]] virtual bool observe_changes() const = 0;
        virtual void set_observe_changes(bool value) = 0;
    };
} // namespace maui::controls
