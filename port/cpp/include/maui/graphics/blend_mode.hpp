#pragma once
// maui::graphics::blend_mode  <=  Microsoft.Maui.Graphics.BlendMode
// The compositing mode used when an object is rendered on top of existing content. Ported from
// src/Graphics/src/Graphics/BlendMode.cs — same members, same order (Normal first / default).

#include <cstdint>

namespace maui::graphics
{
    enum class blend_mode : std::uint8_t
    {
        normal = 0,
        multiply,
        screen,
        overlay,
        darken,
        lighten,
        color_dodge,
        color_burn,
        soft_light,
        hard_light,
        difference,
        exclusion,
        hue,
        saturation,
        color,
        luminosity,
        clear,
        copy,
        source_in,
        source_out,
        source_atop,
        destination_over,
        destination_in,
        destination_out,
        destination_atop,
        xor_, // C# Xor ("xor" is a C++ alternative token)
        plus_darker,
        plus_lighter
    };
} // namespace maui::graphics
