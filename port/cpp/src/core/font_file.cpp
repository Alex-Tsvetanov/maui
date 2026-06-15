// font_file — the cross-platform FromString parse + FileNameWithExtension. See font_file.hpp.
// Ports Microsoft.Maui.FontFile (src/Core/src/Fonts/FontFile.cs).

#include "maui/core/font_file.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace
{
    // Case-insensitive (ASCII, ordinal) suffix test — C# EndsWith(..., StringComparison.OrdinalIgnoreCase).
    bool ends_with_ignore_case(std::string_view value, std::string_view suffix)
    {
        if (suffix.size() > value.size())
        {
            return false;
        }
        const std::string_view tail = value.substr(value.size() - suffix.size());
        return std::ranges::equal(tail, suffix, [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
    }
} // namespace

namespace maui::core
{
    font_file font_file::from_string(std::string_view input)
    {
        // Split "<family>#<postscript>" — only a '#' at index > 0 counts (C# IndexOfChar('#') > 0). The
        // PostScript half has its spaces stripped (UWP family#postscript references keep spaces in the
        // family but not the PostScript name); a bare family is its own PostScript name.
        const std::size_t hash = input.find('#');
        const bool has_hash = hash != std::string_view::npos && hash > 0;

        std::string post_script_name;
        std::string family_name;
        if (has_hash)
        {
            post_script_name = std::string(input.substr(hash + 1));
            std::erase(post_script_name, ' ');
            family_name = std::string(input.substr(0, hash));
        }
        else
        {
            post_script_name = std::string(input);
            family_name = std::string(input);
        }

        // Peel a trailing supported extension off the family half (C# the foreach over Extensions).
        std::string found_extension;
        for (const std::string_view ext : extensions)
        {
            if (ends_with_ignore_case(family_name, ext))
            {
                found_extension = std::string(ext);
                family_name.resize(family_name.size() - ext.size());
                break;
            }
        }

        font_file result;
        result.file_name_ = std::move(family_name);
        result.extension_ = std::move(found_extension);
        result.post_script_name_ = std::move(post_script_name);
        return result;
    }

    std::string font_file::file_name_with_extension(std::string_view ext) const
    {
        return file_name_ + std::string(ext);
    }

    std::string font_file::file_name_with_extension() const
    {
        return file_name_with_extension(extension_);
    }
} // namespace maui::core
