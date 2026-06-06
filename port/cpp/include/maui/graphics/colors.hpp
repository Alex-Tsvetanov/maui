#pragma once
// maui::graphics::colors  <=  Microsoft.Maui.Graphics.Colors
// The 147 system-defined named colors, ported from src/Graphics/src/Graphics/Colors.cs.
//
// MAUI_GRAPHICS_NAMED_COLORS is the single source of truth: X(snake_name, "lowercasename",
// 0xAARRGGBB). It generates (a) the constants below, (b) the parse() name->color lookup in
// color.cpp, and (c) the "every named color round-trips through parse" test. Keeping all three
// off one macro means they can never drift (the round-trip test guards it regardless).

#include "maui/graphics/color.hpp"

#define MAUI_GRAPHICS_NAMED_COLORS(X)                                                                                  \
    X(alice_blue, "aliceblue", 0xFFF0F8FFu)                                                                            \
    X(antique_white, "antiquewhite", 0xFFFAEBD7u)                                                                      \
    X(aqua, "aqua", 0xFF00FFFFu)                                                                                       \
    X(aquamarine, "aquamarine", 0xFF7FFFD4u)                                                                           \
    X(azure, "azure", 0xFFF0FFFFu)                                                                                     \
    X(beige, "beige", 0xFFF5F5DCu)                                                                                     \
    X(bisque, "bisque", 0xFFFFE4C4u)                                                                                   \
    X(black, "black", 0xFF000000u)                                                                                     \
    X(blanched_almond, "blanchedalmond", 0xFFFFEBCDu)                                                                  \
    X(blue, "blue", 0xFF0000FFu)                                                                                       \
    X(blue_violet, "blueviolet", 0xFF8A2BE2u)                                                                          \
    X(brown, "brown", 0xFFA52A2Au)                                                                                     \
    X(burly_wood, "burlywood", 0xFFDEB887u)                                                                            \
    X(cadet_blue, "cadetblue", 0xFF5F9EA0u)                                                                            \
    X(chartreuse, "chartreuse", 0xFF7FFF00u)                                                                           \
    X(chocolate, "chocolate", 0xFFD2691Eu)                                                                             \
    X(coral, "coral", 0xFFFF7F50u)                                                                                     \
    X(cornflower_blue, "cornflowerblue", 0xFF6495EDu)                                                                  \
    X(cornsilk, "cornsilk", 0xFFFFF8DCu)                                                                               \
    X(crimson, "crimson", 0xFFDC143Cu)                                                                                 \
    X(cyan, "cyan", 0xFF00FFFFu)                                                                                       \
    X(dark_blue, "darkblue", 0xFF00008Bu)                                                                              \
    X(dark_cyan, "darkcyan", 0xFF008B8Bu)                                                                              \
    X(dark_goldenrod, "darkgoldenrod", 0xFFB8860Bu)                                                                    \
    X(dark_gray, "darkgray", 0xFFA9A9A9u)                                                                              \
    X(dark_green, "darkgreen", 0xFF006400u)                                                                            \
    X(dark_grey, "darkgrey", 0xFFA9A9A9u)                                                                              \
    X(dark_khaki, "darkkhaki", 0xFFBDB76Bu)                                                                            \
    X(dark_magenta, "darkmagenta", 0xFF8B008Bu)                                                                        \
    X(dark_olive_green, "darkolivegreen", 0xFF556B2Fu)                                                                 \
    X(dark_orange, "darkorange", 0xFFFF8C00u)                                                                          \
    X(dark_orchid, "darkorchid", 0xFF9932CCu)                                                                          \
    X(dark_red, "darkred", 0xFF8B0000u)                                                                                \
    X(dark_salmon, "darksalmon", 0xFFE9967Au)                                                                          \
    X(dark_sea_green, "darkseagreen", 0xFF8FBC8Fu)                                                                     \
    X(dark_slate_blue, "darkslateblue", 0xFF483D8Bu)                                                                   \
    X(dark_slate_gray, "darkslategray", 0xFF2F4F4Fu)                                                                   \
    X(dark_slate_grey, "darkslategrey", 0xFF2F4F4Fu)                                                                   \
    X(dark_turquoise, "darkturquoise", 0xFF00CED1u)                                                                    \
    X(dark_violet, "darkviolet", 0xFF9400D3u)                                                                          \
    X(deep_pink, "deeppink", 0xFFFF1493u)                                                                              \
    X(deep_sky_blue, "deepskyblue", 0xFF00BFFFu)                                                                       \
    X(dim_gray, "dimgray", 0xFF696969u)                                                                                \
    X(dim_grey, "dimgrey", 0xFF696969u)                                                                                \
    X(dodger_blue, "dodgerblue", 0xFF1E90FFu)                                                                          \
    X(firebrick, "firebrick", 0xFFB22222u)                                                                             \
    X(floral_white, "floralwhite", 0xFFFFFAF0u)                                                                        \
    X(forest_green, "forestgreen", 0xFF228B22u)                                                                        \
    X(fuchsia, "fuchsia", 0xFFFF00FFu)                                                                                 \
    X(gainsboro, "gainsboro", 0xFFDCDCDCu)                                                                             \
    X(ghost_white, "ghostwhite", 0xFFF8F8FFu)                                                                          \
    X(gold, "gold", 0xFFFFD700u)                                                                                       \
    X(goldenrod, "goldenrod", 0xFFDAA520u)                                                                             \
    X(gray, "gray", 0xFF808080u)                                                                                       \
    X(green, "green", 0xFF008000u)                                                                                     \
    X(green_yellow, "greenyellow", 0xFFADFF2Fu)                                                                        \
    X(grey, "grey", 0xFF808080u)                                                                                       \
    X(honeydew, "honeydew", 0xFFF0FFF0u)                                                                               \
    X(hot_pink, "hotpink", 0xFFFF69B4u)                                                                                \
    X(indian_red, "indianred", 0xFFCD5C5Cu)                                                                            \
    X(indigo, "indigo", 0xFF4B0082u)                                                                                   \
    X(ivory, "ivory", 0xFFFFFFF0u)                                                                                     \
    X(khaki, "khaki", 0xFFF0E68Cu)                                                                                     \
    X(lavender, "lavender", 0xFFE6E6FAu)                                                                               \
    X(lavender_blush, "lavenderblush", 0xFFFFF0F5u)                                                                    \
    X(lawn_green, "lawngreen", 0xFF7CFC00u)                                                                            \
    X(lemon_chiffon, "lemonchiffon", 0xFFFFFACDu)                                                                      \
    X(light_blue, "lightblue", 0xFFADD8E6u)                                                                            \
    X(light_coral, "lightcoral", 0xFFF08080u)                                                                          \
    X(light_cyan, "lightcyan", 0xFFE0FFFFu)                                                                            \
    X(light_goldenrod_yellow, "lightgoldenrodyellow", 0xFFFAFAD2u)                                                     \
    X(light_gray, "lightgray", 0xFFD3D3D3u)                                                                            \
    X(light_green, "lightgreen", 0xFF90EE90u)                                                                          \
    X(light_grey, "lightgrey", 0xFFD3D3D3u)                                                                            \
    X(light_pink, "lightpink", 0xFFFFB6C1u)                                                                            \
    X(light_salmon, "lightsalmon", 0xFFFFA07Au)                                                                        \
    X(light_sea_green, "lightseagreen", 0xFF20B2AAu)                                                                   \
    X(light_sky_blue, "lightskyblue", 0xFF87CEFAu)                                                                     \
    X(light_slate_gray, "lightslategray", 0xFF778899u)                                                                 \
    X(light_slate_grey, "lightslategrey", 0xFF778899u)                                                                 \
    X(light_steel_blue, "lightsteelblue", 0xFFB0C4DEu)                                                                 \
    X(light_yellow, "lightyellow", 0xFFFFFFE0u)                                                                        \
    X(lime, "lime", 0xFF00FF00u)                                                                                       \
    X(lime_green, "limegreen", 0xFF32CD32u)                                                                            \
    X(linen, "linen", 0xFFFAF0E6u)                                                                                     \
    X(magenta, "magenta", 0xFFFF00FFu)                                                                                 \
    X(maroon, "maroon", 0xFF800000u)                                                                                   \
    X(medium_aquamarine, "mediumaquamarine", 0xFF66CDAAu)                                                              \
    X(medium_blue, "mediumblue", 0xFF0000CDu)                                                                          \
    X(medium_orchid, "mediumorchid", 0xFFBA55D3u)                                                                      \
    X(medium_purple, "mediumpurple", 0xFF9370D8u)                                                                      \
    X(medium_sea_green, "mediumseagreen", 0xFF3CB371u)                                                                 \
    X(medium_slate_blue, "mediumslateblue", 0xFF7B68EEu)                                                               \
    X(medium_spring_green, "mediumspringgreen", 0xFF00FA9Au)                                                           \
    X(medium_turquoise, "mediumturquoise", 0xFF48D1CCu)                                                                \
    X(medium_violet_red, "mediumvioletred", 0xFFC71585u)                                                               \
    X(midnight_blue, "midnightblue", 0xFF191970u)                                                                      \
    X(mint_cream, "mintcream", 0xFFF5FFFAu)                                                                            \
    X(misty_rose, "mistyrose", 0xFFFFE4E1u)                                                                            \
    X(moccasin, "moccasin", 0xFFFFE4B5u)                                                                               \
    X(navajo_white, "navajowhite", 0xFFFFDEADu)                                                                        \
    X(navy, "navy", 0xFF000080u)                                                                                       \
    X(old_lace, "oldlace", 0xFFFDF5E6u)                                                                                \
    X(olive, "olive", 0xFF808000u)                                                                                     \
    X(olive_drab, "olivedrab", 0xFF6B8E23u)                                                                            \
    X(orange, "orange", 0xFFFFA500u)                                                                                   \
    X(orange_red, "orangered", 0xFFFF4500u)                                                                            \
    X(orchid, "orchid", 0xFFDA70D6u)                                                                                   \
    X(pale_goldenrod, "palegoldenrod", 0xFFEEE8AAu)                                                                    \
    X(pale_green, "palegreen", 0xFF98FB98u)                                                                            \
    X(pale_turquoise, "paleturquoise", 0xFFAFEEEEu)                                                                    \
    X(pale_violet_red, "palevioletred", 0xFFD87093u)                                                                   \
    X(papaya_whip, "papayawhip", 0xFFFFEFD5u)                                                                          \
    X(peach_puff, "peachpuff", 0xFFFFDAB9u)                                                                            \
    X(peru, "peru", 0xFFCD853Fu)                                                                                       \
    X(pink, "pink", 0xFFFFC0CBu)                                                                                       \
    X(plum, "plum", 0xFFDDA0DDu)                                                                                       \
    X(powder_blue, "powderblue", 0xFFB0E0E6u)                                                                          \
    X(purple, "purple", 0xFF800080u)                                                                                   \
    X(red, "red", 0xFFFF0000u)                                                                                         \
    X(rosy_brown, "rosybrown", 0xFFBC8F8Fu)                                                                            \
    X(royal_blue, "royalblue", 0xFF4169E1u)                                                                            \
    X(saddle_brown, "saddlebrown", 0xFF8B4513u)                                                                        \
    X(salmon, "salmon", 0xFFFA8072u)                                                                                   \
    X(sandy_brown, "sandybrown", 0xFFF4A460u)                                                                          \
    X(sea_green, "seagreen", 0xFF2E8B57u)                                                                              \
    X(sea_shell, "seashell", 0xFFFFF5EEu)                                                                              \
    X(sienna, "sienna", 0xFFA0522Du)                                                                                   \
    X(silver, "silver", 0xFFC0C0C0u)                                                                                   \
    X(sky_blue, "skyblue", 0xFF87CEEBu)                                                                                \
    X(slate_blue, "slateblue", 0xFF6A5ACDu)                                                                            \
    X(slate_gray, "slategray", 0xFF708090u)                                                                            \
    X(slate_grey, "slategrey", 0xFF708090u)                                                                            \
    X(snow, "snow", 0xFFFFFAFAu)                                                                                       \
    X(spring_green, "springgreen", 0xFF00FF7Fu)                                                                        \
    X(steel_blue, "steelblue", 0xFF4682B4u)                                                                            \
    X(tan, "tan", 0xFFD2B48Cu)                                                                                         \
    X(teal, "teal", 0xFF008080u)                                                                                       \
    X(thistle, "thistle", 0xFFD8BFD8u)                                                                                 \
    X(tomato, "tomato", 0xFFFF6347u)                                                                                   \
    X(transparent, "transparent", 0x00000000u)                                                                         \
    X(turquoise, "turquoise", 0xFF40E0D0u)                                                                             \
    X(violet, "violet", 0xFFEE82EEu)                                                                                   \
    X(wheat, "wheat", 0xFFF5DEB3u)                                                                                     \
    X(white, "white", 0xFFFFFFFFu)                                                                                     \
    X(white_smoke, "whitesmoke", 0xFFF5F5F5u)                                                                          \
    X(yellow, "yellow", 0xFFFFFF00u)                                                                                   \
    X(yellow_green, "yellowgreen", 0xFF9ACD32u)

namespace maui::graphics::colors
{

#define MAUI_GRAPHICS__DEFINE_COLOR(name, str, argb)                                                                   \
    inline const ::maui::graphics::color name = ::maui::graphics::color::from_uint(argb);
    MAUI_GRAPHICS_NAMED_COLORS(MAUI_GRAPHICS__DEFINE_COLOR)
#undef MAUI_GRAPHICS__DEFINE_COLOR

} // namespace maui::graphics::colors
