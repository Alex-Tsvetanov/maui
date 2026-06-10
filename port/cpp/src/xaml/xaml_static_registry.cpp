// maui::xaml::xaml_static_registry — the {x:Static} member table (xaml_static_registry.hpp).
#include "maui/xaml/xaml_static_registry.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace maui::xaml
{
    xaml_static_registry& xaml_static_registry::instance()
    {
        static xaml_static_registry registry;
        return registry;
    }

    void xaml_static_registry::register_member(std::string member, member_fn value)
    {
        members_.insert_or_assign(std::move(member), std::move(value));
    }

    const xaml_static_registry::member_fn* xaml_static_registry::find(std::string_view member) const
    {
        const auto it = members_.find(member);
        return it == members_.end() ? nullptr : &it->second;
    }
} // namespace maui::xaml
