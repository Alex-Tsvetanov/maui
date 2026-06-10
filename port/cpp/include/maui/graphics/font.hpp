#pragma once
// maui::graphics::font  <=  Microsoft.Maui.Graphics.Font  (+ IFont, FontStyleType, FontWeights)
//
// The drawing-layer font descriptor used by i_canvas (distinct from maui::core::font, the port of
// Microsoft.Maui.Font — the higher-level control-facing primitive). Ported from
// src/Graphics/src/Graphics/Font.cs + IFont.cs + FontStyleType.cs.
//
// Deliberate deviation (recorded in port/STATUS.md): C#'s IFont interface is collapsed into this
// value type — Font (a struct) is its only first-party implementation, and value semantics are what
// the canvas op recording needs (no polymorphic font kinds exist in MAUI.Graphics). font_style_type
// and the font_weights constants are a tight cluster with font (PROFILE §3).
//
// An empty name() stands in for C#'s null Name (same convention as maui::core::font's family()).
// Out-of-line definitions live in src/graphics/font.cpp.

#include <cstdint>
#include <string>

namespace maui::graphics
{
    // Microsoft.Maui.Graphics.FontStyleType.
    enum class font_style_type : std::uint8_t
    {
        normal = 0,
        italic = 1,
        oblique = 2
    };

    // Microsoft.Maui.Graphics.FontWeights — the standard weight constants.
    namespace font_weights
    {
        inline constexpr int default_weight = -1; // C# Default ("default" is a C++ keyword)
        inline constexpr int thin = 100;
        inline constexpr int extra_light = 200;
        inline constexpr int ultra_light = 200;
        inline constexpr int light = 300;
        inline constexpr int semi_light = 400;
        inline constexpr int normal = 400;
        inline constexpr int regular = 400;
        inline constexpr int medium = 500;
        inline constexpr int demi_bold = 600;
        inline constexpr int semi_bold = 600;
        inline constexpr int bold = 700;
        inline constexpr int extra_bold = 800;
        inline constexpr int ultra_bold = 800;
        inline constexpr int black = 900;
        inline constexpr int heavy = 900;
        inline constexpr int extra_black = 950;
        inline constexpr int ultra_black = 950;
    } // namespace font_weights

    class font
    {
    public:
        // C# Font.Default — null name, Normal weight/style. (Font's parameterless struct default has
        // weight 0; MAUI only ever surfaces Default/DefaultBold or the explicit ctor, so the port's
        // default ctor mirrors Font.Default — the observable default everywhere.)
        font() = default;
        // C# Font(string name, int weight = Normal, FontStyleType styleType = Normal).
        explicit font(std::string name, int weight = font_weights::normal,
                      font_style_type style_type = font_style_type::normal);

        // C# Font.Default / Font.DefaultBold.
        [[nodiscard]] static font default_font();
        [[nodiscard]] static font default_bold();

        // C# IFont.Name — the font family or file name; empty == C# null (default font).
        [[nodiscard]] const std::string& name() const;
        // C# IFont.Weight.
        [[nodiscard]] int weight() const;
        // C# IFont.StyleType.
        [[nodiscard]] font_style_type style_type() const;

        // C# Font.IsDefault — string.IsNullOrEmpty(Name).
        [[nodiscard]] bool is_default() const;

        // C# Font.Equals(IFont): same style + weight + case-insensitive (ordinal) name.
        [[nodiscard]] bool equals(const font& other) const;

        friend bool operator==(const font& a, const font& b)
        {
            return a.equals(b);
        }
        friend bool operator!=(const font& a, const font& b)
        {
            return !a.equals(b);
        }

    private:
        std::string name_;
        int weight_ = font_weights::normal;
        font_style_type style_type_ = font_style_type::normal;
    };
} // namespace maui::graphics
