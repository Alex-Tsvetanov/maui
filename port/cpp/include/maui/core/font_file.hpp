#pragma once
// maui::core::font_file  <=  Microsoft.Maui.FontFile
//
// Parses a font family / filename string into its parts (filename, extension, PostScript name). Ported
// from src/Core/src/Fonts/FontFile.cs (the cross-platform FromString parse + FileNameWithExtension +
// the supported-extensions list). Pure value logic — no native dependency, fully headless-testable; the
// font_manager's CleanseFontName uses it to look an alias up in the registrar by its filename variants.
//
// FromString splits "<family>#<postscript>" (a UWP-style family#postscript reference — spaces are stripped
// from the PostScript half), then peels a trailing ".ttf"/".otf" extension off the family half. A bare
// family with no '#' uses the whole string as both the filename and the PostScript name.

#include <array>
#include <string>
#include <string_view>

namespace maui::core
{
    class font_file
    {
    public:
        // C# FontFile.Extensions — the supported font-file extensions (with the leading dot).
        static constexpr std::array<std::string_view, 2> extensions{".ttf", ".otf"};

        // C# FontFile.FromString: parse `input` (a filename or font-family name) into its parts.
        [[nodiscard]] static font_file from_string(std::string_view input);

        // C# FontFile.FileName — the family/file name WITHOUT the extension.
        [[nodiscard]] const std::string& file_name() const noexcept
        {
            return file_name_;
        }
        // C# FontFile.Extension — the peeled-off extension (with the dot), or empty if none.
        [[nodiscard]] const std::string& extension() const noexcept
        {
            return extension_;
        }
        // C# FontFile.PostScriptName — the PostScript name (the '#'-suffix, spaces stripped, or the input).
        [[nodiscard]] const std::string& post_script_name() const noexcept
        {
            return post_script_name_;
        }

        // C# FontFile.FileNameWithExtension(extension) — "<file_name><ext>".
        [[nodiscard]] std::string file_name_with_extension(std::string_view ext) const;
        // C# FontFile.FileNameWithExtension() — "<file_name><extension_>".
        [[nodiscard]] std::string file_name_with_extension() const;

    private:
        std::string file_name_;
        std::string extension_;
        std::string post_script_name_;
    };
} // namespace maui::core
