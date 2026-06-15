#pragma once
// maui::core::font_registrar  <=  Microsoft.Maui.FontRegistrar
//
// The concrete font registry: native-font registrations keyed by filename + alias, plus a lookup cache.
// Ported from src/Core/src/Fonts/FontRegistrar.cs (the cross-platform Register/GetFont over the native-font
// dictionary + the _fontLookupCache memo). Cross-platform + headless-testable: the alias→filename mapping
// is pure value logic the font_manager resolves through.
//
// DEVIATIONS vs C# (see i_font_registrar.hpp):
//   * The embedded-font path (Register(.., Assembly) + IEmbeddedFontLoader copying the resource stream out)
//     is omitted — the port has no managed assemblies. Only the native-font registration (Register(filename,
//     alias)) and the alias→filename lookup are modeled. get_font returns the REGISTERED filename for a key
//     (the C# LoadNativeAppFont returns the loader result; with no loader the filename is the resolved name).
//   * The lookup cache stores the last resolution per key (C# _fontLookupCache) so a repeat lookup is O(1).

#include <string>
#include <string_view>
#include <unordered_map>

#include "maui/core/i_font_registrar.hpp"

namespace maui::core
{
    class font_registrar final : public i_font_registrar
    {
    public:
        font_registrar() = default;

        // C# FontRegistrar.Register(filename, alias): store the native font under `filename`, and — when
        // `alias` is non-empty — also under `alias` (both keys map to `filename`).
        void register_font(std::string filename, std::string alias) override;

        // C# FontRegistrar.GetFont(font): the registered filename for the key `font` (alias or filename),
        // or empty if unregistered. Memoized in the lookup cache (including the empty "miss" result).
        [[nodiscard]] std::string get_font(std::string_view font) override;

    private:
        // C# _nativeFonts (key → filename). The C# value tuple also carries the alias, which GetFont never
        // reads back, so the port stores only the resolved filename.
        std::unordered_map<std::string, std::string> native_fonts_;
        // C# _fontLookupCache (key → resolved filename, or "" for a cached miss). std::string is the value;
        // a present entry (even empty) means "already resolved".
        std::unordered_map<std::string, std::string> lookup_cache_;
    };

    // The process-wide default registrar (the C++ stand-in for the DI-registered IFontRegistrar singleton).
    // The default font_manager resolves aliases through it; a host registers app fonts here at startup. Same
    // "process-wide default + lazy use" seam as default_image_source_service_registry() (PROFILE §6).
    font_registrar& default_font_registrar();
} // namespace maui::core
