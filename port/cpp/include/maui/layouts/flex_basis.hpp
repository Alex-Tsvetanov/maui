#pragma once
// maui::layouts::flex_basis  <=  Microsoft.Maui.Layouts.FlexBasis
//
// The per-child initial main-axis size for a FlexLayout: either Auto (size to content), an absolute
// length, or a length relative to the parent (a fraction in [0, 1]). Ported from
// src/Core/src/Layouts/FlexEnums.cs (the FlexBasis struct). A value type; the implicit float ctor
// mirrors C#'s `implicit operator FlexBasis(float)`. The relative-length range + non-negative checks
// match the C# constructor's ArgumentException guards.

#include <stdexcept>

namespace maui::layouts
{
    class flex_basis
    {
    public:
        // Auto basis (the default): size the child to its content along the main axis.
        constexpr flex_basis() = default;

        // An absolute length (is_relative=false) or a parent-relative fraction (is_relative=true, in [0,1]).
        // C#: throws on a negative length, and on a relative length > 1.
        constexpr explicit flex_basis(float length, bool is_relative = false)
            : length_(length), is_relative_(is_relative)
        {
            if (length < 0)
            {
                throw std::invalid_argument("flex_basis length should be a positive value");
            }
            if (is_relative && length > 1)
            {
                throw std::invalid_argument("relative flex_basis length should be in [0, 1]");
            }
            is_length_ = !is_relative;
        }

        // C#: implicit operator FlexBasis(float length) — an absolute length (deliberately implicit,
        // mirroring the C# conversion; the same convention thickness(double) uses).
        constexpr flex_basis(float length) : flex_basis(length, false)
        {
        }

        [[nodiscard]] constexpr float length() const
        {
            return length_;
        }
        [[nodiscard]] constexpr bool is_relative() const
        {
            return is_relative_;
        }
        // Auto when neither an absolute length nor a relative fraction was set.
        [[nodiscard]] constexpr bool is_auto() const
        {
            return !is_length_ && !is_relative_;
        }

        // The shared Auto value (C# FlexBasis.Auto).
        static const flex_basis auto_value;

        [[nodiscard]] constexpr bool operator==(const flex_basis& other) const
        {
            return is_length_ == other.is_length_ && is_relative_ == other.is_relative_ && length_ == other.length_;
        }

    private:
        float length_ = 0;
        bool is_length_ = false;
        bool is_relative_ = false;
    };

    inline const flex_basis flex_basis::auto_value{};
} // namespace maui::layouts
