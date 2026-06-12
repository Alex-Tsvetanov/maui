#pragma once
// maui::controls::i_item_collection  <=  the C# `IEnumerable ItemsSource` face (+ IList fast paths)
// of Microsoft.Maui.Controls.ItemsView.ItemsSource.
//
// The type-erased view of one items source the control stores and the handler-side
// i_items_view_source family (items_view_source.hpp <= IItemsViewSource) consumes. The typed glue is
// item_collection<T>:
//   - over a shared observable_collection<T> (the core range-op flavor): live — changed() exposes the
//     collection's event;
//   - over a plain std::vector<T>: a fixed snapshot — changed() is null (the C# array ItemsSource).
//
// Grouping (C#'s "IsGrouped: the source is an IEnumerable of IEnumerables") is structural here, not
// duck-typed: a grouped source is an item_collection<std::shared_ptr<grouping>>, where each grouping
// carries the group's key object (the BindingContext of the group header/footer templates — C# binds
// them to the group itself, e.g. `class AnimalGroup : List<Animal> { string Name; }`; the port splits
// the "list" half from the "key" half) plus its nested erased collection. group_items(index) is the
// reflection-free substitute for C#'s `item is IEnumerable and not string` group test: non-null only
// when the element at `index` is a group.
//
// Items cross the seam as boxed_item (boxed_item.hpp): at(index) boxes the element — sharing the
// pointee for shared_ptr element types (reference semantics), copying otherwise (value semantics,
// equality through T's operator==).

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::controls
{
    class i_item_collection
    {
    public:
        virtual ~i_item_collection() = default;

        [[nodiscard]] virtual std::size_t count() const = 0;
        [[nodiscard]] virtual boxed_item at(std::size_t index) const = 0;
        // IndexOf via boxed_item::equals; -1 when absent.
        [[nodiscard]] virtual int index_of(const boxed_item& item) const = 0;
        // The live change notification (INotifyCollectionChanged); null for a snapshot source.
        [[nodiscard]] virtual maui::core::event<const maui::core::collection_changed_args&>* changed() = 0;
        // The nested collection when the element at `index` is a group; null otherwise (header note).
        [[nodiscard]] virtual std::shared_ptr<i_item_collection> group_items(std::size_t index) const = 0;

    protected:
        i_item_collection() = default;
        i_item_collection(const i_item_collection&) = default;
        i_item_collection(i_item_collection&&) = default;
        i_item_collection& operator=(const i_item_collection&) = default;
        i_item_collection& operator=(i_item_collection&&) = default;
    };

    // One group of a grouped items source (header note): the key object templates bind against plus
    // the nested items. Declared before item_collection<T> so its group_items specialization can see it.
    class grouping
    {
    public:
        grouping(boxed_item key, std::shared_ptr<i_item_collection> items)
            : key_(std::move(key)), items_(std::move(items))
        {
        }

        [[nodiscard]] const boxed_item& key() const
        {
            return key_;
        }
        [[nodiscard]] const std::shared_ptr<i_item_collection>& items() const
        {
            return items_;
        }

    private:
        boxed_item key_;
        std::shared_ptr<i_item_collection> items_;
    };

    using grouping_ptr = std::shared_ptr<grouping>;

    namespace items_detail
    {
        template <class T> struct shared_ptr_element
        {
            static constexpr bool value = false;
        };
        template <class U> struct shared_ptr_element<std::shared_ptr<U>>
        {
            static constexpr bool value = true;
            using type = U;
        };
    } // namespace items_detail

    // The typed adapter (header note). For a grouped source instantiate with T = grouping_ptr: at()
    // then boxes each group's KEY (the binding context the group templates consume) and group_items()
    // exposes the nested collection.
    template <class T> class item_collection final : public i_item_collection
    {
    public:
        // Live flavor: the collection's own event is the change feed.
        explicit item_collection(std::shared_ptr<maui::core::observable_collection<T>> source)
            : source_(std::move(source)), observable_(true)
        {
        }
        // Snapshot flavor: a fixed vector (no change notifications).
        explicit item_collection(std::vector<T> items)
            : source_(std::make_shared<maui::core::observable_collection<T>>(std::move(items))), observable_(false)
        {
        }

        [[nodiscard]] std::size_t count() const override
        {
            return source_->size();
        }

        [[nodiscard]] boxed_item at(std::size_t index) const override
        {
            const T& element = source_->at(index);
            if constexpr (std::is_same_v<T, grouping_ptr>)
            {
                return element ? element->key() : boxed_item{};
            }
            else
            {
                return boxed_item::of(element);
            }
        }

        [[nodiscard]] int index_of(const boxed_item& item) const override
        {
            const std::size_t total = source_->size();
            for (std::size_t index = 0; index < total; ++index)
            {
                if (at(index) == item)
                {
                    return static_cast<int>(index);
                }
            }
            return -1;
        }

        [[nodiscard]] maui::core::event<const maui::core::collection_changed_args&>* changed() override
        {
            return observable_ ? &source_->collection_changed : nullptr;
        }

        [[nodiscard]] std::shared_ptr<i_item_collection> group_items(std::size_t index) const override
        {
            if constexpr (std::is_same_v<T, grouping_ptr>)
            {
                const grouping_ptr& element = source_->at(index);
                return element ? element->items() : nullptr;
            }
            else
            {
                (void)index;
                return nullptr;
            }
        }

        // The underlying typed collection (the control-side typed accessor; the seam itself stays erased).
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<T>>& source() const
        {
            return source_;
        }

    private:
        std::shared_ptr<maui::core::observable_collection<T>> source_;
        bool observable_;
    };

    // Convenience builders (the typed entries items_view's set_items_source funnels through).
    template <class T>
    [[nodiscard]] std::shared_ptr<i_item_collection> make_item_collection(
        std::shared_ptr<maui::core::observable_collection<T>> source)
    {
        return source ? std::make_shared<item_collection<T>>(std::move(source)) : nullptr;
    }
    template <class T> [[nodiscard]] std::shared_ptr<i_item_collection> make_item_collection(std::vector<T> items)
    {
        return std::make_shared<item_collection<T>>(std::move(items));
    }

    // Build one group: key object + nested typed items (live or snapshot).
    template <class TKey, class TItem>
    [[nodiscard]] grouping_ptr make_grouping(std::shared_ptr<TKey> key,
                                             std::shared_ptr<maui::core::observable_collection<TItem>> items)
    {
        return std::make_shared<grouping>(boxed_item::of(std::move(key)), make_item_collection(std::move(items)));
    }
    template <class TKey, class TItem> [[nodiscard]] grouping_ptr make_grouping(TKey key, std::vector<TItem> items)
    {
        return std::make_shared<grouping>(boxed_item::of(std::move(key)), make_item_collection(std::move(items)));
    }
} // namespace maui::controls
