#pragma once
// maui::core::grid_length  <=  Microsoft.Maui.GridLength
//
// The size of a Grid row/column: an Absolute device-independent length, an Auto (fit-content) length,
// or a Star (proportional-weight) length. A readonly value type. Ported from
// src/Core/src/Primitives/GridLength.cs. (C#'s `Auto`/`Star` statics are `automatic()`/`star()` here.)

#include "maui/core/grid_unit_type.hpp"

namespace maui::core
{
    class grid_length
    {
    public:
        grid_length() = default; // default(GridLength): value 0, Absolute

        // Implicit (intentional, like C#'s `operator GridLength(double)`): an absolute length.
        grid_length(double value);
        grid_length(double value, grid_unit_type type);

        [[nodiscard]] double value() const
        {
            return value_;
        }
        [[nodiscard]] grid_unit_type unit_type() const
        {
            return type_;
        }
        [[nodiscard]] bool is_absolute() const
        {
            return type_ == grid_unit_type::absolute;
        }
        [[nodiscard]] bool is_auto() const
        {
            return type_ == grid_unit_type::automatic;
        }
        [[nodiscard]] bool is_star() const
        {
            return type_ == grid_unit_type::star;
        }

        // Ready-to-use Auto / Star lengths (value is 1; ignored for Auto, the weight for Star).
        [[nodiscard]] static grid_length automatic()
        {
            return {1.0, grid_unit_type::automatic};
        }
        [[nodiscard]] static grid_length star()
        {
            return {1.0, grid_unit_type::star};
        }

        friend bool operator==(const grid_length& a, const grid_length& b);
        friend bool operator!=(const grid_length& a, const grid_length& b);

    private:
        double value_ = 0;
        grid_unit_type type_ = grid_unit_type::absolute;
    };
} // namespace maui::core
