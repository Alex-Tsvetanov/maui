// Implementation of property_mapper_base — the non-generic storage/chaining/dispatch behind the typed
// property_mapper<Virtual,Handler>. See property_mapper.hpp for the design and the C# source mapping.

#include "maui/core/property_mapper.hpp"

#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"

namespace maui::core
{
    void property_mapper_base::set_chained(std::vector<property_mapper_base*> chained)
    {
        chained_ = std::move(chained);
    }

    void property_mapper_base::set_property_core(std::string key, action act)
    {
        for (auto& entry : entries_)
        {
            if (entry.first == key)
            {
                entry.second = std::move(act); // replace in place — keep the key's position
                return;
            }
        }
        entries_.emplace_back(std::move(key), std::move(act));
    }

    const property_mapper_base::action* property_mapper_base::get_property(std::string_view key) const
    {
        for (const auto& entry : entries_)
        {
            if (entry.first == key)
            {
                return &entry.second;
            }
        }
        for (auto* chain : chained_)
        {
            if (const action* found = chain->get_property(key))
            {
                return found;
            }
        }
        return nullptr;
    }

    std::vector<std::string> property_mapper_base::keys() const
    {
        std::vector<std::string> ordered;
        std::unordered_set<std::string> seen;
        const auto add_key = [&ordered, &seen](const std::string& key) {
            if (seen.insert(key).second)
            {
                ordered.push_back(key);
            }
        };
        // Chained keys first, in reverse chain order (matching C# GetKeys).
        for (const auto* chain : std::views::reverse(chained_))
        {
            for (const auto& key : chain->keys())
            {
                add_key(key);
            }
        }
        for (const auto& entry : entries_)
        {
            add_key(entry.first);
        }
        return ordered;
    }

    void property_mapper_base::update_property(i_element_handler& handler, i_element& view, std::string_view key) const
    {
        if (const action* act = get_property(key); act != nullptr && *act)
        {
            (*act)(handler, view);
        }
    }

    void property_mapper_base::update_properties(i_element_handler& handler, i_element& view) const
    {
        for (const auto& key : keys())
        {
            update_property(handler, view, key);
        }
    }
} // namespace maui::core
