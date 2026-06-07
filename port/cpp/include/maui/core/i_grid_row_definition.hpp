#pragma once
// maui::core::i_grid_row_definition  <=  Microsoft.Maui.IGridRowDefinition
// One Grid row's height. Ported from src/Core/src/Core/IGridRowDefinition.cs.

#include "maui/core/grid_length.hpp"

namespace maui::core
{
    class i_grid_row_definition
    {
    public:
        virtual ~i_grid_row_definition() = default;

        [[nodiscard]] virtual grid_length height() const = 0;

    protected:
        i_grid_row_definition() = default;
        i_grid_row_definition(const i_grid_row_definition&) = default;
        i_grid_row_definition(i_grid_row_definition&&) = default;
        i_grid_row_definition& operator=(const i_grid_row_definition&) = default;
        i_grid_row_definition& operator=(i_grid_row_definition&&) = default;
    };
} // namespace maui::core
