// maui::xaml::markup_extension_registry — the name -> factory store (i_markup_extension.hpp).
#include "maui/xaml/i_markup_extension.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace maui::xaml
{
    markup_extension_registry& markup_extension_registry::instance()
    {
        static markup_extension_registry registry;
        return registry;
    }

    void markup_extension_registry::register_extension(std::string name, markup_extension_factory factory)
    {
        // Register the plain name and tolerate the "<name>Extension" spelling (C# strips/append-matches
        // the suffix during type resolution). The suffixed alias shares the factory.
        factories_.insert_or_assign(name + "Extension", factory);
        factories_.insert_or_assign(std::move(name), std::move(factory));
    }

    const markup_extension_factory* markup_extension_registry::find(std::string_view name) const
    {
        const auto it = factories_.find(name);
        return it == factories_.end() ? nullptr : &it->second;
    }
} // namespace maui::xaml
