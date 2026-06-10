#pragma once
// maui::controls::i_multi_value_converter  <=  Microsoft.Maui.Controls.IMultiValueConverter
//
// Combines a multi_binding's source values into one target value (convert) and splits a target value
// back into per-binding source values (convert_back), over the boxed representation (std::any; an
// EMPTY any is null). Sentinel semantics, straight from the C# documentation table:
//   convert:      do_nothing -> no target update; unset_value -> the multi_binding's FallbackValue;
//                 null (empty any) -> TargetNullValue
//   convert_back: nullopt (C# null array) -> no source updates at all; per-element do_nothing /
//                 unset_value -> that source binding is skipped
// As with i_value_converter, System.Type becomes maui::core::type_tag and CultureInfo is dropped.

#include <any>
#include <optional>
#include <vector>

#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class i_multi_value_converter
    {
    public:
        i_multi_value_converter() = default;
        i_multi_value_converter(const i_multi_value_converter&) = delete;
        i_multi_value_converter(i_multi_value_converter&&) = delete;
        i_multi_value_converter& operator=(const i_multi_value_converter&) = delete;
        i_multi_value_converter& operator=(i_multi_value_converter&&) = delete;
        virtual ~i_multi_value_converter() = default;

        [[nodiscard]] virtual std::any convert(const std::vector<std::any>& values, maui::core::type_tag target_type,
                                               const std::any& parameter) = 0;

        [[nodiscard]] virtual std::optional<std::vector<std::any>> convert_back(
            const std::any& value, const std::vector<maui::core::type_tag>& target_types,
            const std::any& parameter) = 0;
    };
} // namespace maui::controls
