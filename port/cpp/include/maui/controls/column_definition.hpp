#pragma once
// maui::controls::column_definition  <=  Microsoft.Maui.Controls.ColumnDefinition
//
// Defines the width of one column in a grid. Ported from src/Controls/src/Core/ColumnDefinition.cs. The
// C# type is a BindableObject whose Width bindable property defaults to GridLength.Star and raises
// SizeChanged on change. Here it is a small value type implementing i_grid_column_definition: a plain
// grid_length width_ + a setter. (As with row_definition, the C# bindable Width drives a layout
// re-measure via SizeChanged; that invalidation path is not yet wired in the port, so a plain value +
// setter is the simple shape — promoting width_ to a bindable_property is the natural follow-up.)

#include "maui/core/grid_length.hpp"
#include "maui/core/i_grid_column_definition.hpp"

namespace maui::controls
{
    class column_definition : public maui::core::i_grid_column_definition
    {
    public:
        // Default width is Star, matching ColumnDefinition()'s GridLength.Star default in C#.
        column_definition() = default;
        explicit column_definition(maui::core::grid_length width) : width_(width)
        {
        }

        [[nodiscard]] maui::core::grid_length width() const override
        {
            return width_;
        }
        void set_width(maui::core::grid_length value)
        {
            width_ = value;
        }

    private:
        maui::core::grid_length width_ = maui::core::grid_length::star();
    };
} // namespace maui::controls
