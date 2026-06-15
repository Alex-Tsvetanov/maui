#pragma once
// maui::core::i_font_manager  <=  Microsoft.Maui.IFontManager (+ IFontManager.iOS DefaultFont / GetFont)
//
// Resolves an abstract maui::core::font (family + size + weight + slant + auto-scaling) to the platform's
// native font object. Ported from src/Core/src/Fonts/IFontManager.cs (DefaultFontSize) +
// IFontManager.iOS.cs (DefaultFont + GetFont(font, defaultFontSize)). An abstract class (PROFILE §11 —
// the font service holds it by reference and resolves through it at runtime; the apple/ios font_manager
// is the concrete UIFont/NSFont implementation, the headless one a deterministic fake).
//
// `get_font` returns an OPAQUE native handle (a `UIFont*`/`NSFont*`, reinterpreted as `void*`). The handle
// is OWNED by the manager's internal font cache (C# FontManager._fonts) and stays valid for the manager's
// lifetime, so a caller borrows it transiently (e.g. the font image service draws the glyph in the same
// call). The apple/ios .mm reinterprets it back via (__bridge); headless returns a non-null sentinel.
//
// DEVIATION vs C#: GetFont returns void* (not UIFont) so the contract is backend-agnostic in a header
// includable from cross-platform code; the platform .mm bridges it. The DefaultFont accessor is exposed
// the same way (a native handle).

#include "maui/core/font.hpp"

namespace maui::core
{
    class i_font_manager
    {
    public:
        virtual ~i_font_manager() = default;

        // C# IFontManager.DefaultFontSize — the OS default font size (apple: UIFont.systemFontSize; the
        // headless fake reports a fixed value; C# Standard reports -1).
        [[nodiscard]] virtual double default_font_size() const = 0;

        // C# IFontManager.DefaultFont — the OS default native font handle (apple: UIFont.SystemFontOfSize
        // (SystemFontSize)). Owned by the manager; a non-null sentinel headless.
        [[nodiscard]] virtual void* default_font() = 0;

        // C# IFontManager.GetFont(font, defaultFontSize): resolve `value` to the native font (a UIFont*/
        // NSFont* as void*). `default_size` supplies the size when the font carries none (<= 0); 0 means
        // "use DefaultFont.PointSize / DefaultFontSize". The result is cached + owned by the manager.
        [[nodiscard]] virtual void* get_font(const font& value, double default_size = 0) = 0;

    protected:
        i_font_manager() = default;
        i_font_manager(const i_font_manager&) = default;
        i_font_manager(i_font_manager&&) = default;
        i_font_manager& operator=(const i_font_manager&) = default;
        i_font_manager& operator=(i_font_manager&&) = default;
    };
} // namespace maui::core
