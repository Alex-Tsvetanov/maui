// maui::xaml::xaml_converter_registry — dispatch + the process-wide default (xaml_converter_registry.hpp).
#include "maui/xaml/xaml_converter_registry.hpp"

#include <any>
#include <string>

#include "maui/core/type_tag.hpp"

namespace maui::xaml
{
    bool xaml_converter_registry::has_converter(maui::core::type_tag target_type) const
    {
        return converters_.contains(target_type);
    }

    std::any xaml_converter_registry::convert(maui::core::type_tag target_type, const std::string& text) const
    {
        const auto it = converters_.find(target_type);
        return it == converters_.end() ? std::any{} : it->second(text);
    }

    xaml_converter_registry& default_xaml_converter_registry()
    {
        static xaml_converter_registry registry;
        return registry;
    }
} // namespace maui::xaml
