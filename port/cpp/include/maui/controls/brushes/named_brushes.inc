// named_brushes.inc — expands the named-brush list through the caller-defined MAUI_CONTROLS_NAMED_BRUSH
// macro (declaration in brush.hpp, definition in brush.cpp). Single source of truth: the SAME
// MAUI_GRAPHICS_NAMED_COLORS(X) table that generates graphics::colors, so the brush statics and the color
// constants can never drift (the C# Brush.* statics are exactly `new(Colors.<Name>)`).
//
// The includer must (1) have included maui/graphics/colors.hpp at FILE scope first (this file is included
// inside a class body / function body, so it must NOT pull headers), and (2) #define
// MAUI_CONTROLS_NAMED_BRUSH(name, str, argb) before including, then #undef it afterward. NOT #pragma once
// — it is included twice (declaration + definition).

MAUI_GRAPHICS_NAMED_COLORS(MAUI_CONTROLS_NAMED_BRUSH)
