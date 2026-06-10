// maui::xaml::xaml_type_registry — lookups + the process-wide default registry (xaml_type_registry.hpp).
#include "maui/xaml/xaml_type_registry.hpp"

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"

namespace maui::xaml
{
    const xaml_type_registry::registration* xaml_type_registry::find(std::string_view name, xaml_namespace ns) const
    {
        const name_map& entries = entries_for(ns);
        const auto it = entries.find(std::string{name});
        return it == entries.end() ? nullptr : &it->second;
    }

    std::shared_ptr<maui::core::bindable_object> xaml_type_registry::create(std::string_view name,
                                                                            xaml_namespace ns) const
    {
        const registration* entry = find(name, ns);
        return entry == nullptr ? nullptr : entry->create();
    }

    bool xaml_type_registry::is_registered(std::string_view name, xaml_namespace ns) const
    {
        return find(name, ns) != nullptr;
    }

    xaml_type_registry& default_xaml_type_registry()
    {
        static xaml_type_registry registry;
        return registry;
    }
} // namespace maui::xaml
