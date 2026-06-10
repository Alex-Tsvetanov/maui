// maui::xaml::name_scope  <=  Microsoft.Maui.Controls.Internals.NameScope
// (src/Controls/src/Core/Internals/NameScope.cs). See name_scope.hpp for the placement deviation.
#include "maui/xaml/name_scope.hpp"

#include <any>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace maui::xaml
{
    void name_scope::register_name(std::string name, std::any scoped_element)
    {
        // NameScope.RegisterName: ArgumentException($"An element with the key '{name}' already
        // exists in NameScope", nameof(name)).
        if (names_.contains(name))
        {
            throw std::invalid_argument("An element with the key '" + name + "' already exists in NameScope");
        }
        names_.emplace(std::move(name), std::move(scoped_element));
        // (C#'s reverse _values map exists only for the VS Live Visual Tree's NameOf — not ported.)
    }

    const std::any* name_scope::find_by_name(std::string_view name) const
    {
        const auto found = names_.find(name);
        return found != names_.end() ? &found->second : nullptr;
    }

    void name_scope::unregister_name(std::string_view name)
    {
        // NameScope.UnregisterName's guards, in C#'s order (the null-name case cannot arise here).
        if (name.empty())
        {
            throw std::invalid_argument("name was provided as empty string.");
        }
        const auto found = names_.find(name);
        if (found == names_.end())
        {
            throw std::invalid_argument("name provided had not been registered.");
        }
        names_.erase(found);
    }
} // namespace maui::xaml
