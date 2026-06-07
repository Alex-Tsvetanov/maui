// Implementation of command_mapper_base — the non-generic storage/chaining/dispatch behind the typed
// command_mapper<Virtual,Handler>. See command_mapper.hpp for the design and the C# source mapping.

#include "maui/core/command_mapper.hpp"

#include <any>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"

namespace maui::core
{
    void command_mapper_base::set_chained(command_mapper_base* chained)
    {
        chained_ = chained;
    }

    void command_mapper_base::set_command_core(std::string key, action act)
    {
        for (auto& entry : entries_)
        {
            if (entry.first == key)
            {
                entry.second = std::move(act);
                return;
            }
        }
        entries_.emplace_back(std::move(key), std::move(act));
    }

    const command_mapper_base::action* command_mapper_base::get_command(std::string_view key) const
    {
        for (const auto& entry : entries_)
        {
            if (entry.first == key)
            {
                return &entry.second;
            }
        }
        if (chained_ != nullptr)
        {
            return chained_->get_command(key);
        }
        return nullptr;
    }

    void command_mapper_base::invoke(i_element_handler& handler, i_element& view, std::string_view command,
                                     const std::any& args) const
    {
        if (const action* act = get_command(command); act != nullptr && *act)
        {
            (*act)(handler, view, args);
        }
    }
} // namespace maui::core
