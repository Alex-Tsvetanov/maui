// font_manager — the cross-platform half: ctor/dtor, the font cache + size resolution in get_font, and
// cleanse_font_name (the registrar lookup). The native create_font body + default_font / default_font_size
// live in the per-backend partial (font_manager.{mm,cpp}). See font_manager.hpp. Ports the cross-platform
// portions of Microsoft.Maui.FontManager (FontManager.iOS.cs GetFont/GetFontSize/CleanseFontName).

#include "maui/core/font_manager.hpp"

#include <cmath>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/font.hpp"
#include "maui/core/font_file.hpp"
#include "maui/core/font_registrar.hpp" // default_font_registrar()
#include "maui/core/i_font_registrar.hpp"

namespace maui::core
{
    namespace
    {
        // The device-independent floor for size resolution when no positive size is available from the font,
        // the caller's default, or the backend's default font size (e.g. headless's -1). The common 12pt
        // system size; on apple/ios default_font_size() is always positive so this is never reached.
        constexpr double k_fallback_font_size = 12.0;
    } // namespace

    font_manager::font_manager() : registrar_(&default_font_registrar())
    {
    }

    font_manager::font_manager(i_font_registrar& registrar) : registrar_(&registrar)
    {
    }

    font_manager::~font_manager()
    {
        // Release every cached native font + the default font (RAII; the disposers CFRelease on apple,
        // no-op headless). The vector clear runs the cache disposers via cache_entry's move_only_function.
        for (auto& entry : cache_)
        {
            if (entry.dispose)
            {
                entry.dispose();
            }
        }
        if (default_dispose_)
        {
            default_dispose_();
        }
    }

    double font_manager::resolve_font_size(const font& value, double default_size) const
    {
        // C# GetFontSize: size <= 0 || NaN → (defaultFontSize > 0 ? defaultFontSize : DefaultFont.PointSize);
        // else the font's own size. On apple/ios DefaultFont.PointSize == DefaultFontSize, so the last
        // fallback uses default_font_size(); a non-positive backend value (headless's -1) is floored to a
        // fixed device-independent size so size resolution always yields a positive value.
        if (value.size() <= 0 || std::isnan(value.size()))
        {
            if (default_size > 0)
            {
                return default_size;
            }
            const double backend_default = default_font_size();
            return backend_default > 0 ? backend_default : k_fallback_font_size;
        }
        return value.size();
    }

    void* font_manager::get_font(const font& value, double default_size)
    {
        // C# GetFont(font, defaultFont, factory): resolve the effective size, fold it onto the font when it
        // differs, then GetOrAdd the cache keyed by the (size-resolved) font.
        const double size = resolve_font_size(value, default_size);
        font key = (size != value.size()) ? value.with_size(size) : value;

        for (const auto& entry : cache_)
        {
            if (entry.key == key)
            {
                return entry.handle;
            }
        }

        created_font created = create_font(key);
        void* const handle = created.handle;
        cache_.push_back(cache_entry{.key = std::move(key), .handle = handle, .dispose = std::move(created.dispose)});
        return handle;
    }

    std::string font_manager::cleanse_font_name(const std::string& family) const
    {
        // C# CleanseFontName: first the alias (registrar.GetFont(fontName)).
        if (std::string alias = registrar_->get_font(family); !alias.empty())
        {
            return alias;
        }

        const font_file file = font_file::from_string(family);

        // With an explicit extension, look up "<name><ext>"; else probe each supported extension. A hit
        // returns the registered filePath; a miss falls through to the PostScript name (C#'s final return).
        if (!file.extension().empty())
        {
            if (std::string file_path = registrar_->get_font(file.file_name_with_extension()); !file_path.empty())
            {
                return file_path;
            }
        }
        else
        {
            for (const std::string_view ext : font_file::extensions)
            {
                if (std::string file_path = registrar_->get_font(file.file_name_with_extension(ext));
                    !file_path.empty())
                {
                    return file_path;
                }
            }
        }

        return file.post_script_name();
    }

    font_manager& default_font_manager()
    {
        static font_manager manager;
        return manager;
    }
} // namespace maui::core
