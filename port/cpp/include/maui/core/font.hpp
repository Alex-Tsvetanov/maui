#pragma once
// maui::core::font  <=  Microsoft.Maui.Font  (+ FontWeight, FontSlant)
// An immutable font descriptor (family, size, weight, slant, auto-scaling). Ported from
// src/Core/src/Primitives/Font.cs. font_weight/font_slant are a tight cluster with font (§3).
// An empty family() stands in for C#'s null Family. (C# stores a 0 "unset" weight reported as
// regular; since 0 and regular are observationally identical everywhere, we just default to regular.)

#include <cstdint>
#include <string>
#include <string_view>

namespace maui::core
{
    enum class font_weight : std::uint16_t
    {
        thin = 100,
        ultralight = 200,
        light = 300,
        regular = 400,
        medium = 500,
        semibold = 600,
        bold = 700,
        heavy = 800,
        black = 900,
    };

    enum class font_slant : std::uint8_t
    {
        normal = 0, // C# FontSlant.Default ("default" is a C++ keyword)
        italic = 1,
        oblique = 2,
    };

    class font
    {
    public:
        font() = default; // the default font (no family, size 0, normal slant, regular weight)

        [[nodiscard]] const std::string& family() const; // empty == no family (C# null)
        [[nodiscard]] double size() const;
        [[nodiscard]] font_slant slant() const;
        [[nodiscard]] font_weight weight() const;
        [[nodiscard]] bool auto_scaling_enabled() const;
        [[nodiscard]] bool is_default() const;

        // Builders (C# With*): return a copy with one field changed.
        [[nodiscard]] font with_size(double new_size) const;
        [[nodiscard]] font with_slant(font_slant new_slant) const;
        [[nodiscard]] font with_weight(font_weight new_weight) const;
        [[nodiscard]] font with_weight(font_weight new_weight, font_slant new_slant) const;
        [[nodiscard]] font with_auto_scaling(bool enabled) const;

        // Factories (C# OfSize / SystemFontOfSize / SystemFontOfWeight / Default).
        [[nodiscard]] static font of_size(std::string_view name, double size, font_weight weight = font_weight::regular,
                                          font_slant slant = font_slant::normal, bool enable_scaling = true);
        [[nodiscard]] static font system_font_of_size(double size, font_weight weight = font_weight::regular,
                                                      font_slant slant = font_slant::normal,
                                                      bool enable_scaling = true);
        [[nodiscard]] static font system_font_of_weight(font_weight weight, font_slant slant = font_slant::normal,
                                                        bool enable_scaling = true);
        [[nodiscard]] static font default_font();

        [[nodiscard]] std::string to_string() const;

    private:
        font(std::string family, double size, font_slant slant, font_weight weight, bool enable_scaling);

        std::string family_;
        double size_ = 0;
        font_slant slant_ = font_slant::normal;
        font_weight weight_ = font_weight::regular;
        bool disable_scaling_ = false;
    };

    bool operator==(const font& a, const font& b);
    bool operator!=(const font& a, const font& b);
} // namespace maui::core
