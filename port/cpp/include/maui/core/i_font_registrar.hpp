#pragma once
// maui::core::i_font_registrar  <=  Microsoft.Maui.IFontRegistrar
//
// Tracks the fonts registered with the application — a filename plus an optional alias by which the same
// font can also be referenced. Ported from src/Core/src/Fonts/IFontRegistrar.cs. The font_manager's
// CleanseFontName resolves a family name to a registered font through get_font (alias → filename), so a
// font registered under an alias resolves to its real file name when the manager builds the native font.
//
// An abstract class (PROFILE §11 — the manager holds it by reference and resolves through it at runtime).
//
// DEVIATION vs C#: C#'s Register(filename, alias, Assembly) carries the assembly the embedded font lives
// in, and GetFont copies the embedded resource stream out via IEmbeddedFontLoader. The port has no managed
// assemblies / embedded resources, so the assembly overload + the embedded-loader copy step are omitted;
// get_font resolves the registered name directly (the alias→filename mapping), which is the behavior the
// manager depends on. Documented, not stubbed.

#include <string>
#include <string_view>

namespace maui::core
{
    class i_font_registrar
    {
    public:
        virtual ~i_font_registrar() = default;

        // C# IFontRegistrar.Register(filename, alias): register `filename`, and — when `alias` is non-empty
        // — also register it under `alias` (so either name resolves to the font). An empty alias registers
        // only the filename.
        virtual void register_font(std::string filename, std::string alias) = 0;

        // C# IFontRegistrar.GetFont(font): the registered font name for the key `font` (an alias or a
        // filename), or empty if nothing is registered under that key. (C# returns null; the port uses an
        // empty string for "not found", consistent with the font/family empty == null convention.)
        [[nodiscard]] virtual std::string get_font(std::string_view font) = 0;

    protected:
        i_font_registrar() = default;
        i_font_registrar(const i_font_registrar&) = default;
        i_font_registrar(i_font_registrar&&) = default;
        i_font_registrar& operator=(const i_font_registrar&) = default;
        i_font_registrar& operator=(i_font_registrar&&) = default;
    };
} // namespace maui::core
