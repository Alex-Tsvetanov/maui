#pragma once
// maui::core::i_grid_column_definition  <=  Microsoft.Maui.IGridColumnDefinition
// One Grid column's width. Ported from src/Core/src/Core/IGridColumnDefinition.cs.

#include "maui/core/grid_length.hpp"

namespace maui::core
{
    class i_grid_column_definition
    {
    public:
        virtual ~i_grid_column_definition() = default;

        [[nodiscard]] virtual grid_length width() const = 0;

    protected:
        i_grid_column_definition() = default;
        i_grid_column_definition(const i_grid_column_definition&) = default;
        i_grid_column_definition(i_grid_column_definition&&) = default;
        i_grid_column_definition& operator=(const i_grid_column_definition&) = default;
        i_grid_column_definition& operator=(i_grid_column_definition&&) = default;
    };
} // namespace maui::core
