// maui::core::thickness (thickness.hpp).
#include "maui/core/thickness.hpp"

#include <cmath>

namespace maui::core
{
    const thickness thickness::zero{};

    thickness::thickness(double uniform_size)
        : left(uniform_size), top(uniform_size), right(uniform_size), bottom(uniform_size)
    {
    }
    thickness::thickness(double horizontal_size, double vertical_size)
        : left(horizontal_size), top(vertical_size), right(horizontal_size), bottom(vertical_size)
    {
    }
    thickness::thickness(double left_value, double top_value, double right_value, double bottom_value)
        : left(left_value), top(top_value), right(right_value), bottom(bottom_value)
    {
    }

    double thickness::horizontal_thickness() const
    {
        return left + right;
    }
    double thickness::vertical_thickness() const
    {
        return top + bottom;
    }
    bool thickness::is_empty() const
    {
        return left == 0.0 && top == 0.0 && right == 0.0 && bottom == 0.0;
    }
    bool thickness::is_nan() const
    {
        return std::isnan(left) && std::isnan(top) && std::isnan(right) && std::isnan(bottom);
    }

    bool operator==(const thickness& a, const thickness& b)
    {
        return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
    }
    bool operator!=(const thickness& a, const thickness& b)
    {
        return !(a == b);
    }
    thickness operator+(const thickness& value, double addend)
    {
        return {value.left + addend, value.top + addend, value.right + addend, value.bottom + addend};
    }
    thickness operator+(const thickness& a, const thickness& b)
    {
        return {a.left + b.left, a.top + b.top, a.right + b.right, a.bottom + b.bottom};
    }
    thickness operator-(const thickness& value, double subtrahend)
    {
        return {value.left - subtrahend, value.top - subtrahend, value.right - subtrahend, value.bottom - subtrahend};
    }
} // namespace maui::core
