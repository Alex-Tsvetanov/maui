// maui::xaml — the runtime {Binding} applier (xaml_runtime_bindings.hpp): the port of
// ApplyPropertiesVisitor.TrySetBinding, registered into the xaml_binding_applier seam.
#include "maui/xaml/xaml_runtime_bindings.hpp"

#include <format>
#include <string>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_binding_applier.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // TrySetBinding: resolve the attribute to its bindable descriptor and SetBinding the built
        // binding. Failures fall to TrySetPropertyValue's catch-all message (the same one literal
        // attribute misses produce — LoaderTests.TestUnknownPropertyShouldThrow's shape), exactly
        // where C#'s TrySetBinding returning false lands.
        void apply_runtime_binding(const xaml_property_registry& properties, maui::core::bindable_object& target,
                                   maui::core::type_tag target_type, const std::string& xaml_name,
                                   const binding_request& request, int line_number, int line_position)
        {
            const xaml_property_registry::property_entry* entry = properties.find(target_type, xaml_name);
            auto* element = dynamic_cast<maui::controls::element*>(&target);
            if (entry == nullptr || entry->bindable_name.empty() || element == nullptr || request.instance == nullptr)
            {
                throw xaml_parse_exception(
                    std::format("Cannot assign property \"{}\": Property does not exist, or is not assignable, "
                                "or mismatching type between value and property",
                                xaml_name),
                    line_number, line_position);
            }
            element->set_binding(std::string{entry->bindable_name}, request.instance);
        }
    } // namespace

    void register_runtime_bindings()
    {
        set_xaml_binding_applier(&apply_runtime_binding);
    }
} // namespace maui::xaml
