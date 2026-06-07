// maui::core::grid_length — validation + equality. See grid_length.hpp + GridLength.cs.

#include "maui/core/grid_length.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

#include "maui/core/grid_unit_type.hpp"

namespace maui::core
{
    grid_length::grid_length(double value) : grid_length(value, grid_unit_type::absolute)
    {
    }

    grid_length::grid_length(double value, grid_unit_type type) : value_(value), type_(type)
    {
        if (value < 0 || std::isnan(value))
        {
            throw std::invalid_argument("grid_length: value is less than 0 or is not a number");
        }
    }

    bool operator==(const grid_length& a, const grid_length& b)
    {
        return a.type_ == b.type_ && std::abs(a.value_ - b.value_) < std::numeric_limits<double>::epsilon();
    }

    bool operator!=(const grid_length& a, const grid_length& b)
    {
        return !(a == b);
    }
} // namespace maui::core
