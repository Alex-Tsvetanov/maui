#pragma once
// maui::controls::row_definition  <=  Microsoft.Maui.Controls.RowDefinition
//
// Defines the height of one row in a grid. Ported from src/Controls/src/Core/RowDefinition.cs. The C#
// type is a BindableObject whose Height bindable property defaults to GridLength.Star and raises
// SizeChanged on change. Here it is a small value type implementing i_grid_row_definition: a plain
// grid_length height_ + a setter. (The C# bindable Height drives a layout re-measure via SizeChanged;
// that invalidation path is not yet wired in the port, so a plain value + setter is the simple, honest
// shape — promoting height_ to a bindable_property is the natural follow-up once grid re-measure exists.)

#include "maui/core/grid_length.hpp"
#include "maui/core/i_grid_row_definition.hpp"

namespace maui::controls
{
    class row_definition : public maui::core::i_grid_row_definition
    {
    public:
        // Default height is Star, matching RowDefinition()'s GridLength.Star default in C#.
        row_definition() = default;
        explicit row_definition(maui::core::grid_length height) : height_(height)
        {
        }

        [[nodiscard]] maui::core::grid_length height() const override
        {
            return height_;
        }
        void set_height(maui::core::grid_length value)
        {
            height_ = value;
        }

    private:
        maui::core::grid_length height_ = maui::core::grid_length::star();
    };
} // namespace maui::controls
