#pragma once
// maui::core::i_grid_layout  <=  Microsoft.Maui.IGridLayout
//
// A layout that arranges children in rows and columns. Ported from src/Core/src/Core/IGridLayout.cs.
// The C# IReadOnlyList<IGrid*Definition> collections are exposed here as count + indexed accessor
// (the idiomatic C++ read-only-list shape, as with i_container). The Grid.Row/Column/Span attached
// properties are exposed as get_* queries keyed on the child view.

#include "maui/core/i_grid_column_definition.hpp"
#include "maui/core/i_grid_row_definition.hpp"
#include "maui/core/i_layout.hpp"

namespace maui::core
{
    class i_grid_layout : public i_layout
    {
    public:
        [[nodiscard]] virtual int row_definition_count() const = 0;
        [[nodiscard]] virtual const i_grid_row_definition& row_definition_at(int index) const = 0;
        [[nodiscard]] virtual int column_definition_count() const = 0;
        [[nodiscard]] virtual const i_grid_column_definition& column_definition_at(int index) const = 0;

        [[nodiscard]] virtual double row_spacing() const = 0;
        [[nodiscard]] virtual double column_spacing() const = 0;

        // The Grid.Row/Column/RowSpan/ColumnSpan attached properties for a child.
        [[nodiscard]] virtual int get_row(const i_view& view) const = 0;
        [[nodiscard]] virtual int get_row_span(const i_view& view) const = 0;
        [[nodiscard]] virtual int get_column(const i_view& view) const = 0;
        [[nodiscard]] virtual int get_column_span(const i_view& view) const = 0;
    };
} // namespace maui::core
