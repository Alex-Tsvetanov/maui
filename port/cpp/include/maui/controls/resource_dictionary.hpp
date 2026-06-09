#pragma once
// maui::controls::resource_dictionary  <=  Microsoft.Maui.Controls.ResourceDictionary
//
// A string-keyed store of resource objects (styles, colors, strings, …) with merged dictionaries — the
// code-first equivalent of a XAML <ResourceDictionary>. C#'s ResourceDictionary stores every value as
// System.Object; the typed C++ port keeps each value in a std::any (the same boundary-confined erasure
// `setter` already uses to feed bindable_object::apply_setter), so a value can be retrieved typed via
// get<T>() and a DynamicResource can hand the raw std::any straight to apply_setter. (The binding_context
// box's shared_ptr<void>+type_tag pattern is for non-copyable shared ownership; resources are copyable
// values + shared_ptr<style>, for which std::any is the right tool and plugs into the existing seam.)
//
// Lookup precedence (try_get): the immediate inner dictionary first, then merged dictionaries scanned
// LAST-to-FIRST (later merges win — ResourceDictionary.TryGetValue / TryGetMergedDictionaryValue). Count
// and contains_key ignore merged dictionaries (matching C#, which does so for Hot Reload). Adding /
// changing a value, or adding a merged dictionary, fires values_changed with the affected key/value pairs
// (so a bound DynamicResource re-applies) — ResourceDictionary.OnValuesChanged.
//
// Implicit styles (add(style) with no class) are stored under the style's target type key, produced by
// implicit_style_key(type_tag); style classes (add(style) with a class) accumulate under a class-prefixed
// key. Scope: the in-memory store + merged dictionaries + the change event. The XAML Source URI, system
// resources, style sheets, and the ConditionalWeakTable instance cache are out of scope (STATUS.md).

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/core/event.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class style; // implicit / class styles are stored as resources, keyed by type / class

    // The prefix C# uses for class-style resource keys (Style.StyleClassPrefix). A class style is stored
    // under (k_style_class_prefix + class-name) so style-class selection can look it up.
    inline constexpr std::string_view k_style_class_prefix = "Microsoft.Maui.Controls.StyleClass.";

    // The resource key an implicit (TargetType-keyed) style is stored under. C# uses Type.FullName; the
    // reflection-free port uses the type_tag's stable hash rendered as a hex string, so the key is a
    // process-stable string usable in the same string-keyed dictionary as named resources.
    [[nodiscard]] std::string implicit_style_key(maui::core::type_tag target_type);

    // One (key, value) pair delivered by values_changed (ResourcesChangedEventArgs.Values). The value is a
    // borrowed view (valid for the duration of the notification) into the dictionary's stored std::any.
    struct resource_change
    {
        std::string_view key;
        const std::any* value;
    };

    class resource_dictionary
    {
    public:
        resource_dictionary() = default;
        resource_dictionary(const resource_dictionary&) = delete;
        resource_dictionary(resource_dictionary&&) = delete;
        resource_dictionary& operator=(const resource_dictionary&) = delete;
        resource_dictionary& operator=(resource_dictionary&&) = delete;
        // Destruction is order-safe in BOTH directions: an outer dict that merged this one drops it from its
        // merged list, and this dict's own merged subscriptions are torn down — so neither a destroyed inner
        // nor a destroyed outer leaves a dangling subscription (the merged relationship is non-owning either
        // way).
        ~resource_dictionary();

        // Add a new key. An existing key is rejected (ResourceDictionary.Add throws ArgumentException; the
        // port returns false and leaves the existing value). Fires values_changed on success. Use set() to
        // overwrite.
        bool add(std::string key, std::any value);
        // Overwrite-or-insert (the indexer setter this[key] = value). Always fires values_changed.
        void set(std::string key, std::any value);

        // Add an implicit / class style (ResourceDictionary.Add(Style)): a style with no class is stored
        // under its target-type key; a classed style accumulates into a vector<shared_ptr<style>> under the
        // class-prefixed key. (Defined in the .cpp — needs the full style type.)
        void add(std::shared_ptr<style> value);

        // Whether the IMMEDIATE dictionary contains key (ignores merged dictionaries, like C#).
        [[nodiscard]] bool contains_key(std::string_view key) const;
        // The count of the IMMEDIATE dictionary (ignores merged dictionaries, like C#).
        [[nodiscard]] std::size_t count() const
        {
            return inner_.size();
        }

        // Resolve key through this dictionary then its merged dictionaries (last-to-first). Returns a
        // borrowed pointer to the stored value, or nullptr if absent. The reference is valid until the next
        // mutation of the owning dictionary.
        [[nodiscard]] const std::any* try_get(std::string_view key) const;

        // Typed retrieval: the value at key as T, or nullptr if absent or stored as a different type.
        template <class T> [[nodiscard]] const T* get(std::string_view key) const
        {
            const std::any* value = try_get(key);
            return value != nullptr ? std::any_cast<T>(value) : nullptr;
        }

        // Merged dictionaries (ResourceDictionary.MergedDictionaries) — later entries take precedence on
        // lookup. NON-owning: the caller owns each merged dictionary's lifetime; this dictionary subscribes
        // to its values_changed so a downstream change re-propagates. Adding one fires values_changed for
        // every key it currently contributes (OnValuesChanged(rd.ToArray)).
        void add_merged_dictionary(resource_dictionary& value);
        // Detach every merged dictionary (MergedDictionaries.Clear) — does NOT fire values_changed (C#'s
        // Reset case keeps existing values aligned).
        void clear_merged_dictionaries();
        // The merged dictionaries, as a pointer view (rebuilt on demand from the internal entries).
        [[nodiscard]] const std::vector<resource_dictionary*>& merged_dictionaries() const;

        // Visit every (key, value) this dictionary contributes — its own entries plus all merged ones, in
        // C#'s MergedResources order (merged dictionaries last-to-first, then the inner entries). Used by an
        // element to gather the resources that just became visible when a dictionary or parent is attached.
        void each_merged_resource(const std::function<void(const resource_change&)>& visit) const;

        // Fired when a value is added/changed, or a merged dictionary is added (ResourceDictionary
        // .ValuesChanged). Carries the affected pairs; an element listens to re-apply bound DynamicResources.
        maui::core::event<const std::vector<resource_change>&> values_changed;

    private:
        void on_values_changed(const std::vector<resource_change>& values) const;
        // Detach one merged dictionary: disconnect our subscription to its values_changed and drop the
        // bookkeeping on both sides. Called by clear_merged_dictionaries and by the destructor.
        void detach_merged(resource_dictionary& value);

        // One merged dictionary plus the token of OUR subscription to its values_changed (so we can
        // disconnect using the inner dict's own event — never a dangling scoped_connection that might
        // outlive the publisher). NON-owning.
        struct merged_entry
        {
            resource_dictionary* dictionary;
            maui::core::connection_token token;
        };

        // Ordinal string-keyed store (StringComparer.Ordinal). std::string keys are owned (a resource key
        // may be a built dynamic string, e.g. an implicit-style or class-style key).
        std::unordered_map<std::string, std::any> inner_;
        std::vector<merged_entry> merged_;                      // NON-owning merged dictionaries we draw from
        std::vector<resource_dictionary*> merged_into_;         // dicts that merged THIS one (the back-links)
        mutable std::vector<resource_dictionary*> merged_view_; // cached pointer view for merged_dictionaries()
    };
} // namespace maui::controls
