// font_registrar — the native-font registry + lookup-cache memo. See font_registrar.hpp.
// Ports the cross-platform half of Microsoft.Maui.FontRegistrar (src/Core/src/Fonts/FontRegistrar.cs).

#include "maui/core/font_registrar.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace maui::core
{
    void font_registrar::register_font(std::string filename, std::string alias)
    {
        // C# Register: _nativeFonts[filename] = (filename, alias); and, when the alias is non-empty,
        // _nativeFonts[alias] = (filename, alias). Both keys resolve to the same filename.
        if (!alias.empty())
        {
            native_fonts_[std::move(alias)] = filename;
        }
        native_fonts_[filename] = filename;
    }

    std::string font_registrar::get_font(std::string_view font)
    {
        // C# GetFont: a cached entry (including a cached miss) short-circuits.
        const std::string key(font);
        if (const auto cached = lookup_cache_.find(key); cached != lookup_cache_.end())
        {
            return cached->second;
        }

        // C# LoadNativeAppFont: with no embedded-font loader the registered filename IS the resolved name.
        // A miss caches the empty result (C# _fontLookupCache[font] = null).
        std::string result;
        if (const auto found = native_fonts_.find(key); found != native_fonts_.end())
        {
            result = found->second;
        }
        lookup_cache_[key] = result;
        return result;
    }

    font_registrar& default_font_registrar()
    {
        static font_registrar registrar;
        return registrar;
    }
} // namespace maui::core
