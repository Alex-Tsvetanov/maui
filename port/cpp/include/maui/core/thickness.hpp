#pragma once
// maui::core::thickness  <=  Microsoft.Maui.Thickness
// A margin/padding: four doubles (left, top, right, bottom). Ported from
// src/Core/src/Primitives/Thickness.cs. Public members, so structured bindings replace C#'s
// Deconstruct (`auto [l, t, r, b] = thk;`).
//
// M1 deviation (port/STATUS.md): the implicit Size -> Thickness conversion is omitted (rarely used);
// add it when layout needs it.

namespace maui::core
{
    struct thickness
    {
        double left = 0;
        double top = 0;
        double right = 0;
        double bottom = 0;

        static const thickness zero;

        thickness() = default;
        thickness(double uniform_size); // implicit, mirroring C#'s double -> Thickness operator
        thickness(double horizontal_size, double vertical_size);
        thickness(double left_value, double top_value, double right_value, double bottom_value);

        [[nodiscard]] double horizontal_thickness() const; // left + right
        [[nodiscard]] double vertical_thickness() const;   // top + bottom
        [[nodiscard]] bool is_empty() const;               // all four are zero
        [[nodiscard]] bool is_nan() const;                 // all four are NaN
    };

    bool operator==(const thickness& a, const thickness& b);
    bool operator!=(const thickness& a, const thickness& b);
    thickness operator+(const thickness& value, double addend);
    thickness operator+(const thickness& a, const thickness& b);
    thickness operator-(const thickness& value, double subtrahend);
} // namespace maui::core
