// maui::core::bindable_object — the change-notification base (bindable_object.hpp).
#include "maui/core/bindable_object.hpp"

#include <string_view>

namespace maui::core
{
    void bindable_object::on_property_changed(std::string_view name)
    {
        property_changed.raise(name);
    }

    void bindable_object::on_property_changing(std::string_view name)
    {
        property_changing.raise(name);
    }
} // namespace maui::core
