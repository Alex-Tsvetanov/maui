#pragma once
// maui::core::font_manager  <=  Microsoft.Maui.FontManager (FontManager.iOS.cs / FontManager.Standard.cs)
//
// Resolves a maui::core::font to the platform's native font (a UIFont*/NSFont*), with the embedded-font /
// alias registry (via i_font_registrar) and a font cache. The concrete i_font_manager. Ported from
// FontManager.iOS.cs (CreateFont's family / .SFUI-* system-font / cleansed-name resolution +
// GetFontAttributes weight/slant traits + ApplyScaling) and FontManager.Standard.cs (the headless fake —
// DefaultFontSize = -1). The font image service resolves its glyph's typeface through this manager
// (matching C#'s FontImageSourceService taking an IFontManager), so a registered/aliased family renders
// in the right typeface.
//
// Partial-class split (PROFILE §5): the ctor + the font cache + cleanse_font_name (the registrar lookup)
// are cross-platform here (font_manager.cpp); default_font_size / default_font / get_font's native
// CreateFont body is per backend (src/platform/{apple,ios}/font_manager.mm build a real UIFont/NSFont;
// src/platform/headless/font_manager.cpp returns a deterministic sentinel for the tests).
//
// Ownership (PROFILE §8): the manager OWNS every native font it creates (a retained UIFont*/NSFont* slot
// + its CFRelease disposer), held in the font cache for the manager's lifetime and released in the dtor.
// A caller borrows the handle transiently. The registrar is held by reference (non-owning; must outlive
// the manager) — defaults to the process-wide default_font_registrar().

#include <string>
#include <vector>

#include "maui/core/font.hpp"
#include "maui/core/i_font_manager.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class i_font_registrar;

    class font_manager final : public i_font_manager
    {
    public:
        // Build over the process-wide default_font_registrar() (the common case).
        font_manager();
        // Build over a specific registrar (non-owning; must outlive the manager). Tests register fonts in
        // a local registrar and pass it here to exercise the alias-resolution path deterministically.
        explicit font_manager(i_font_registrar& registrar);

        font_manager(const font_manager&) = delete;
        font_manager& operator=(const font_manager&) = delete;
        font_manager(font_manager&&) = delete;
        font_manager& operator=(font_manager&&) = delete;
        ~font_manager() override;

        // i_font_manager (per-backend native bodies — font_manager.{mm,cpp}).
        [[nodiscard]] double default_font_size() const override;
        [[nodiscard]] void* default_font() override;
        [[nodiscard]] void* get_font(const font& value, double default_size = 0) override;

        // C# FontManager.CleanseFontName: resolve a family name to a registered font name. First the alias
        // (registrar.get_font(name)); then, by the parsed font_file's filename+extension variants; falling
        // back to the PostScript name. Cross-platform (the registrar lookup) — exposed for the per-backend
        // CreateFont + the tests. Returns the PostScript name as the final fallback, like C#.
        [[nodiscard]] std::string cleanse_font_name(const std::string& family) const;

        // The registrar this manager resolves aliases through (the embedded/native font registry).
        [[nodiscard]] i_font_registrar& registrar() const noexcept
        {
            return *registrar_;
        }

    private:
        // The native font CreateFont produces for `value` (per backend). Returns the handle + its disposer;
        // a null handle (impossible in practice — the system font is the floor) yields an empty disposer.
        // const: it builds a fresh native font + resolves through the (const) registrar but does not mutate
        // the manager — get_font owns the cache insert. Per-backend (.mm / headless .cpp).
        struct created_font
        {
            void* handle = nullptr;
            move_only_function<void()> dispose;
        };
        [[nodiscard]] created_font create_font(const font& value) const;

        // The font size CreateFont uses (C# GetFontSize): the font's own size when > 0, else `default_size`
        // when > 0, else the default font's point size. On apple/ios C# DefaultFont.PointSize ==
        // DefaultFontSize (both UIFont.systemFontSize), so this last fallback uses default_font_size();
        // headless's -1 sentinel is floored to a fixed device-independent size. Cross-platform.
        [[nodiscard]] double resolve_font_size(const font& value, double default_size) const;

        struct cache_entry
        {
            font key;
            void* handle = nullptr;
            move_only_function<void()> dispose;
        };

        i_font_registrar* registrar_;
        std::vector<cache_entry> cache_;             // C# _fonts (linear-scan; font caches are tiny)
        void* default_font_ = nullptr;               // C# _defaultFont (lazily created, owned)
        move_only_function<void()> default_dispose_; // releases default_font_ in the dtor
    };

    // The process-wide default font manager (the C++ stand-in for the DI-registered IFontManager singleton),
    // resolving aliases through default_font_registrar(). The font image service resolves through this when
    // none is injected — same "process-wide default + lazy use" seam as default_image_source_service_registry().
    font_manager& default_font_manager();
} // namespace maui::core
