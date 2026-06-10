// maui::xaml — the {Binding} applier hook (xaml_binding_applier.hpp). The default rejects loudly;
// the runtime-binding unit swaps the real TrySetBinding port in via set_xaml_binding_applier.
#include "maui/xaml/xaml_binding_applier.hpp"

#include <format>
#include <string>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // The rejecting default — {Binding} is a loud load failure until the runtime-binding unit
        // registers its applier (the documented M7 deferral channel).
        void reject_binding(const xaml_property_registry& /*properties*/, maui::core::bindable_object& /*target*/,
                            maui::core::type_tag /*target_type*/, const std::string& xaml_name,
                            const binding_request& /*request*/, int line_number, int line_position)
        {
            throw xaml_parse_exception(
                std::format("Cannot set a {{Binding}} on \"{}\": no binding applier is registered (the "
                            "runtime-binding unit provides one; STATUS.md M7 deferrals)",
                            xaml_name),
                line_number, line_position);
        }

        [[nodiscard]] xaml_binding_applier& applier_slot()
        {
            static xaml_binding_applier applier = &reject_binding;
            return applier;
        }
    } // namespace

    const xaml_binding_applier& current_xaml_binding_applier()
    {
        return applier_slot();
    }

    void set_xaml_binding_applier(xaml_binding_applier applier)
    {
        applier_slot() = applier ? std::move(applier) : xaml_binding_applier{&reject_binding};
    }
} // namespace maui::xaml
