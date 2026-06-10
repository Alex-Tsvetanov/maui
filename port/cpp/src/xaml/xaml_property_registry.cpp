// maui::xaml::xaml_property_registry — lookups/application + the process-wide default
// (xaml_property_registry.hpp).
#include "maui/xaml/xaml_property_registry.hpp"

#include <any>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"

namespace maui::xaml
{
    void xaml_property_registry::add_property(maui::core::type_tag type, std::string xaml_name, property_entry entry)
    {
        types_[type].properties.insert_or_assign(std::move(xaml_name), std::move(entry));
    }

    void xaml_property_registry::set_content_property(maui::core::type_tag type, std::string xaml_name)
    {
        types_[type].content_property = std::move(xaml_name);
    }

    void xaml_property_registry::set_add_child(maui::core::type_tag type, add_child_fn add)
    {
        types_[type].add_child = std::move(add);
    }

    void xaml_property_registry::set_child_property_name(maui::core::type_tag type, std::string xaml_name)
    {
        types_[type].child_property = std::move(xaml_name);
    }

    const xaml_property_registry::property_entry* xaml_property_registry::find(maui::core::type_tag type,
                                                                               std::string_view xaml_name) const
    {
        const auto type_it = types_.find(type);
        if (type_it == types_.end())
        {
            return nullptr;
        }
        const auto property_it = type_it->second.properties.find(std::string{xaml_name});
        return property_it == type_it->second.properties.end() ? nullptr : &property_it->second;
    }

    bool xaml_property_registry::try_set(maui::core::type_tag type, maui::core::bindable_object& target,
                                         std::string_view xaml_name, const std::any& value) const
    {
        const property_entry* entry = find(type, xaml_name);
        return entry != nullptr && entry->set(target, value);
    }

    bool xaml_property_registry::try_set_from_text(maui::core::type_tag type, maui::core::bindable_object& target,
                                                   std::string_view xaml_name, const std::string& text,
                                                   const xaml_converter_registry& converters) const
    {
        const property_entry* entry = find(type, xaml_name);
        if (entry == nullptr)
        {
            return false;
        }
        const std::any value = converters.convert(entry->value_type, text);
        return value.has_value() && entry->set(target, value);
    }

    const std::string* xaml_property_registry::content_property(maui::core::type_tag type) const
    {
        const auto it = types_.find(type);
        if (it == types_.end() || it->second.content_property.empty())
        {
            return nullptr;
        }
        return &it->second.content_property;
    }

    bool xaml_property_registry::try_add_child(maui::core::type_tag parent_type, maui::core::bindable_object& parent,
                                               maui::core::bindable_object& child) const
    {
        const auto it = types_.find(parent_type);
        return it != types_.end() && it->second.add_child && it->second.add_child(parent, child);
    }

    bool xaml_property_registry::is_child_property(maui::core::type_tag type, std::string_view xaml_name) const
    {
        const auto it = types_.find(type);
        return it != types_.end() && !it->second.child_property.empty() && it->second.child_property == xaml_name;
    }

    xaml_property_registry& default_xaml_property_registry()
    {
        static xaml_property_registry registry;
        return registry;
    }
} // namespace maui::xaml
