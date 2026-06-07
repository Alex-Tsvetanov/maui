#pragma once
// maui::core::property_mapper  <=  Microsoft.Maui.PropertyMapper / IPropertyMapper
//
// The property-name → update-function table at the heart of the handler recipe (PROFILE §5, CLAUDE
// handler-recipe step 2). A handler's mapper maps each virtual-view property name to a function that
// pushes that property's current value onto the platform view. Mappers CHAIN: a control's mapper falls
// back to the view mapper, which falls back to the element mapper. A key in a nearer mapper overrides a
// farther one, but still runs in the farther one's POSITION (so the container-view mapper keeps running
// first) — matching C#'s GetKeys (chained keys first) / GetProperty (own action wins) split. Ported
// from src/Core/src/PropertyMapper.cs.
//
// Two layers mirror C#'s non-generic PropertyMapper + generic PropertyMapper<TVirtual,THandler>:
//   - property_mapper_base — non-generic storage/chaining/dispatch over (i_element_handler&,i_element&);
//     this is what handlers hold polymorphically and what chaining points at.
//   - property_mapper<Virtual,Handler> — the typed authoring surface: add(name, fn) with a typed
//     fn(Handler&, Virtual&); it down-casts at the boundary (dynamic_cast, mirroring C#'s `is`/cast).
//
// M1 simplifications (documented, behavior-preserving for the handler flow): no merged-mapper cache
// (C#'s SnapshotMappers is a perf optimization; key lists are recomputed each pass), and no
// CanInvokeMappers gate (a platform batching hook — there are no platform mappers yet). Insertion order
// is preserved (entries are a vector, not a hash map) so override positioning matches C#.

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"

namespace maui::core
{
    class property_mapper_base
    {
    public:
        using action = std::function<void(i_element_handler&, i_element&)>;

        property_mapper_base() = default;
        virtual ~property_mapper_base() = default;
        property_mapper_base(const property_mapper_base&) = default;
        property_mapper_base(property_mapper_base&&) = default;
        property_mapper_base& operator=(const property_mapper_base&) = default;
        property_mapper_base& operator=(property_mapper_base&&) = default;

        // C# Chained: the mappers this one falls back to, searched in order. Non-owning (mappers are
        // statics that outlive every handler).
        void set_chained(std::vector<property_mapper_base*> chained);

        // C# GetProperty: this mapper's own action for `key`, else the first chained match, else null.
        [[nodiscard]] const action* get_property(std::string_view key) const;

        // C# GetKeys().Distinct(): chained keys (reverse chain order) first, then own keys, de-duped
        // keeping first occurrence — the update order used by update_properties().
        [[nodiscard]] std::vector<std::string> keys() const;

        // C# UpdateProperty / UpdateProperties.
        void update_property(i_element_handler& handler, i_element& view, std::string_view key) const;
        void update_properties(i_element_handler& handler, i_element& view) const;

    protected:
        // C# SetPropertyCore: add, or replace in place (replacing keeps the key's original position).
        void set_property_core(std::string key, action act);

    private:
        std::vector<std::pair<std::string, action>> entries_;
        std::vector<property_mapper_base*> chained_;
    };

    template <class Virtual, class Handler> class property_mapper : public property_mapper_base
    {
        static_assert(std::is_base_of_v<i_element, Virtual>, "Virtual must derive maui::core::i_element");
        static_assert(std::is_base_of_v<i_element_handler, Handler>,
                      "Handler must derive maui::core::i_element_handler");

    public:
        using typed_action = std::function<void(Handler&, Virtual&)>;
        using entry = std::pair<std::string_view, typed_action>;

        property_mapper() = default;

        explicit property_mapper(std::initializer_list<entry> entries)
        {
            add_all(entries);
        }

        property_mapper(property_mapper_base& chained, std::initializer_list<entry> entries)
        {
            set_chained({&chained});
            add_all(entries);
        }

        // Chain onto another mapper with no own entries (e.g. a handler whose properties all come from
        // a chained text/view mapper).
        explicit property_mapper(property_mapper_base& chained)
        {
            set_chained({&chained});
        }

        // C# Add / this[key] = action.
        void add(std::string key, typed_action action)
        {
            set_property_core(std::move(key), [typed = std::move(action)](i_element_handler& handler, i_element& view) {
                // Mirrors C#'s `if (v is TVirtualView vv) action((TViewHandler)h, vv)`:
                // the view is checked (no-op if it isn't this Virtual); the handler is
                // hard-cast (the reference dynamic_cast throws on a genuine type
                // mismatch, like InvalidCastException). In the normal handler flow both
                // always match — the handler invokes with its own typed view.
                if (auto* typed_view = dynamic_cast<Virtual*>(&view))
                {
                    typed(dynamic_cast<Handler&>(handler), *typed_view);
                }
            });
        }

    private:
        void add_all(std::initializer_list<entry> entries)
        {
            for (const auto& [key, action] : entries)
            {
                add(std::string(key), action);
            }
        }
    };
} // namespace maui::core
