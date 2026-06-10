#pragma once
// maui::core::i_indexable  <=  the C# indexer property (`this[...]`) + DefaultMemberAttribute, as a
//                              contract — the reflection-free analog BindingExpression's indexer parts
//                              ("Path[index]") resolve against.
//
// C# bindings reach `Model[1]` through the source's reflected indexer ("Item", or the name declared by
// [IndexerName]/[DefaultMember]) and observe it through PropertyChanged("Item[1]"). The port has no
// reflection, so a bindable source that wants indexer-path support implements this contract instead:
//   - try_get_item / try_set_item move boxed values (boxed_value.hpp; an EMPTY std::any is null);
//     nullopt/false = the index isn't present (C#'s KeyNotFound/IndexOutOfRange -> unresolved, never
//     a throw);
//   - indexer_name() is the [IndexerName] analog — change notification must raise property_changed
//     with "<indexer_name>[<index>]" (or bare "<indexer_name>"), exactly the names C#'s
//     BindingExpressionPart.PropertyChanged matches;
//   - try_get_item_object exposes an item that is itself a walkable bindable_object (for an indexer
//     mid-path, "Items[0].Name"); the default says "not an object".
// The index argument is the raw text between the path's brackets (C# keeps it unparsed too and lets
// the indexer decide — an int indexer parses it, a string indexer uses it verbatim).

#include <any>
#include <memory>
#include <optional>
#include <string_view>

namespace maui::core
{
    class bindable_object;

    class i_indexable
    {
    public:
        i_indexable() = default;
        i_indexable(const i_indexable&) = delete;
        i_indexable(i_indexable&&) = delete;
        i_indexable& operator=(const i_indexable&) = delete;
        i_indexable& operator=(i_indexable&&) = delete;
        virtual ~i_indexable() = default;

        // The name change notifications use ("Item" unless the type declares otherwise — [IndexerName]).
        [[nodiscard]] virtual std::string_view indexer_name() const
        {
            return "Item";
        }

        // The item at `index`, boxed; nullopt when the index isn't present.
        [[nodiscard]] virtual std::optional<std::any> try_get_item(std::string_view index) const = 0;

        // Store the boxed value at `index`; false when the index/value isn't settable.
        virtual bool try_set_item(std::string_view index, const std::any& value) = 0;

        // The item as a walkable bindable_object node (for an indexer mid-path), or null.
        [[nodiscard]] virtual std::shared_ptr<bindable_object> try_get_item_object(std::string_view index) const
        {
            (void)index;
            return nullptr;
        }
    };
} // namespace maui::core
