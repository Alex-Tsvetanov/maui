// Implementation of handler_registry's non-template members. The factory recording + create_handler<>
// template live in the header; the type_tag lookups live here. See handler_registry.hpp.

#include "maui/core/handler_registry.hpp"

#include <memory>

#include "maui/core/i_element_handler.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    std::unique_ptr<i_element_handler> handler_registry::create_handler(type_tag view_type) const
    {
        const auto found = factories_.find(view_type);
        if (found == factories_.end())
        {
            return nullptr;
        }
        return found->second();
    }

    bool handler_registry::is_registered(type_tag view_type) const
    {
        return factories_.contains(view_type);
    }
} // namespace maui::core
