#pragma once
// maui::graphics::canvas_defaults  <=  Microsoft.Maui.Graphics.CanvasDefaults
// Default values for canvas-related properties. Ported from
// src/Graphics/src/Graphics/CanvasDefaults.cs (a static class of constants — here a namespace).

#include "maui/graphics/color.hpp"
#include "maui/graphics/size_f.hpp"

namespace maui::graphics::canvas_defaults
{
    // C# CanvasDefaults.DefaultShadowColor — black at 50% opacity.
    inline const color default_shadow_color{0.0F, 0.0F, 0.0F, 0.5F};

    // C# CanvasDefaults.DefaultShadowOffset — 5 units in both directions.
    inline const size_f default_shadow_offset{5, 5};

    // C# CanvasDefaults.DefaultShadowBlur.
    inline constexpr float default_shadow_blur = 5;

    // C# CanvasDefaults.DefaultMiterLimit.
    inline constexpr float default_miter_limit = 10;
} // namespace maui::graphics::canvas_defaults
