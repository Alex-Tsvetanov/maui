// maui::controls::resource_dictionary — the string-keyed resource store + merged dictionaries
// (resource_dictionary.hpp). Ported from ResourceDictionary.cs (the in-memory store; XAML Source,
// system resources, and style sheets are out of scope).
#include "maui/controls/resource_dictionary.hpp"

#include <any>
#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/style.hpp"
#include "maui/core/event.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    std::string implicit_style_key(maui::core::type_tag target_type)
    {
        // C# keys implicit styles by Type.FullName; the reflection-free port uses the type_tag's stable
        // hash (a folded-to-one address) so the key is a process-stable string. The "ImplicitStyle:" prefix
        // keeps it distinct from any user resource key.
        return std::format("ImplicitStyle:{:x}", target_type.hash());
    }

    bool resource_dictionary::add(std::string key, std::any value)
    {
        if (inner_.contains(key))
        {
            return false; // C# throws ArgumentException for a duplicate key; the port rejects it.
        }
        auto [it, inserted] = inner_.emplace(std::move(key), std::move(value));
        on_values_changed({resource_change{.key = it->first, .value = &it->second}});
        return true;
    }

    void resource_dictionary::set(std::string key, std::any value)
    {
        auto [it, inserted] = inner_.insert_or_assign(std::move(key), std::move(value));
        on_values_changed({resource_change{.key = it->first, .value = &it->second}});
    }

    void resource_dictionary::add(std::shared_ptr<style> value)
    {
        if (!value)
        {
            return;
        }
        if (value->style_class().empty())
        {
            // Implicit style: keyed by the target type (ResourceDictionary.Add(Style) → TargetType.FullName).
            add(implicit_style_key(value->target_type()), std::any{value});
            return;
        }
        // Class style: accumulate into the vector under the class-prefixed key. A second style with the same
        // class appends (StyleClassAreCorrectlyMerged), so an existing vector is copied + extended.
        std::string key{k_style_class_prefix};
        key += value->style_class();
        std::vector<std::shared_ptr<style>> classes;
        if (const auto* existing = get<std::vector<std::shared_ptr<style>>>(key))
        {
            classes = *existing;
        }
        classes.push_back(std::move(value));
        set(std::move(key), std::any{std::move(classes)});
    }

    bool resource_dictionary::contains_key(std::string_view key) const
    {
        return inner_.contains(std::string{key});
    }

    resource_dictionary::~resource_dictionary()
    {
        // Order-safe teardown. (1) Drop my subscriptions to each inner dict + the back-link it holds to me,
        // so a still-living inner doesn't notify a dead outer. (2) Tell every outer dict that merged ME to
        // forget this one, so a still-living outer doesn't disconnect against my dead values_changed event.
        while (!merged_.empty())
        {
            detach_merged(*merged_.back().dictionary);
        }
        // Drain-style loop: each detach_merged(*this) erases that outer from merged_into_, so a
        // range-for over the vector would increment an invalidated iterator (caught by the MSVC STL's
        // debug iterators on Windows; silent element-shifting on libc++).
        while (!merged_into_.empty())
        {
            merged_into_.back()->detach_merged(*this);
        }
    }

    const std::vector<resource_dictionary*>& resource_dictionary::merged_dictionaries() const
    {
        merged_view_.clear();
        merged_view_.reserve(merged_.size());
        for (const merged_entry& entry : merged_)
        {
            merged_view_.push_back(entry.dictionary);
        }
        return merged_view_;
    }

    const std::any* resource_dictionary::try_get(std::string_view key) const
    {
        if (auto it = inner_.find(std::string{key}); it != inner_.end())
        {
            return &it->second;
        }
        // Merged dictionaries are scanned LAST-to-FIRST so a later merge wins (C#'s
        // TryGetMergedDictionaryValue loops i = Count-1 .. 0).
        for (const merged_entry& entry : std::ranges::reverse_view(merged_))
        {
            if (const std::any* value = entry.dictionary->try_get(key))
            {
                return value;
            }
        }
        return nullptr;
    }

    void resource_dictionary::add_merged_dictionary(resource_dictionary& value)
    {
        // Subscribe to the inner dict's values_changed via its OWN event (storing the token), and register a
        // back-link on it — so destruction in either order tears the relationship down without dangling.
        const maui::core::connection_token token = value.values_changed.connect(
            [this](const std::vector<resource_change>& values) { on_values_changed(values); });
        merged_.push_back(merged_entry{.dictionary = &value, .token = token});
        value.merged_into_.push_back(this);
        // Surface everything the newly-merged dictionary contributes (OnValuesChanged(rd.ToArray())).
        std::vector<resource_change> contributed;
        value.each_merged_resource([&contributed](const resource_change& change) { contributed.push_back(change); });
        if (!contributed.empty())
        {
            on_values_changed(contributed);
        }
    }

    void resource_dictionary::detach_merged(resource_dictionary& value)
    {
        for (auto it = merged_.begin(); it != merged_.end(); ++it)
        {
            if (it->dictionary == &value)
            {
                value.values_changed.disconnect(it->token); // unsubscribe via the inner dict's own event
                merged_.erase(it);
                break;
            }
        }
        std::erase(value.merged_into_, this); // drop the back-link
    }

    void resource_dictionary::clear_merged_dictionaries()
    {
        // C#'s Reset case unsubscribes and clears, but deliberately does NOT fire ValuesChanged (so already
        // resolved DynamicResource values stay put — RemovingMergedRDDoesntTriggersValueChanged).
        while (!merged_.empty())
        {
            detach_merged(*merged_.back().dictionary);
        }
    }

    void resource_dictionary::each_merged_resource(const std::function<void(const resource_change&)>& visit) const
    {
        // MergedResources order: merged dictionaries last-to-first, then the inner entries (so the inner
        // dictionary's own values come out last and therefore win in GetMergedResources' first-write map).
        for (const merged_entry& entry : std::ranges::reverse_view(merged_))
        {
            entry.dictionary->each_merged_resource(visit);
        }
        for (const auto& [key, value] : inner_)
        {
            visit(resource_change{.key = key, .value = &value});
        }
    }

    void resource_dictionary::on_values_changed(const std::vector<resource_change>& values) const
    {
        if (values.empty())
        {
            return;
        }
        values_changed.raise(values);
    }
} // namespace maui::controls
