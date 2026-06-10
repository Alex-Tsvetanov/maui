#pragma once
// maui::controls::i_value_converter  <=  Microsoft.Maui.Controls.IValueConverter
//
// Converts a value as it flows source->target (convert) or target->source (convert_back) through a
// binding, over the port's boxed-value representation (std::any; an EMPTY any is null —
// boxed_value.hpp). Differences from C#, both reflection consequences:
//   - `targetType` becomes a maui::core::type_tag (the target property's value type — compare it
//     against type_tag::of<T>(); there is no System.Type to inspect);
//   - the CultureInfo parameter is dropped (the port converts invariantly; no culture object exists).
// A converter may return the do-nothing sentinel (binding_base.hpp) to suppress the update, exactly
// like C#'s Binding.DoNothing.

#include <any>

#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class i_value_converter
    {
    public:
        i_value_converter() = default;
        i_value_converter(const i_value_converter&) = delete;
        i_value_converter(i_value_converter&&) = delete;
        i_value_converter& operator=(const i_value_converter&) = delete;
        i_value_converter& operator=(i_value_converter&&) = delete;
        virtual ~i_value_converter() = default;

        // Source value -> target value (used by one_way / two_way / one_time flows).
        [[nodiscard]] virtual std::any convert(const std::any& value, maui::core::type_tag target_type,
                                               const std::any& parameter) = 0;

        // Target value -> source value (used by two_way / one_way_to_source flows).
        [[nodiscard]] virtual std::any convert_back(const std::any& value, maui::core::type_tag target_type,
                                                    const std::any& parameter) = 0;
    };
} // namespace maui::controls
