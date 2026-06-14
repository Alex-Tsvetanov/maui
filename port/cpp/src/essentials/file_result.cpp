// file_result - the FileBase-derived accessors: FileName from the path leaf, ContentType from the
// extension, and OpenReadAsync reading the file bytes. The MIME map ports the common image/video
// types the media picker produces.

#include "maui/essentials/file_result.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace maui::storage
{
    namespace detail
    {
        std::string content_type_from_extension(std::string_view extension)
        {
            std::string ext(extension);
            std::ranges::transform(ext, ext.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            // The common image/video MIME types FileSystem.PlatformGetContentType resolves.
            if (ext == "png")
            {
                return "image/png";
            }
            if (ext == "jpg" || ext == "jpeg")
            {
                return "image/jpeg";
            }
            if (ext == "gif")
            {
                return "image/gif";
            }
            if (ext == "bmp")
            {
                return "image/bmp";
            }
            if (ext == "webp")
            {
                return "image/webp";
            }
            if (ext == "heic")
            {
                return "image/heic";
            }
            if (ext == "mp4")
            {
                return "video/mp4";
            }
            if (ext == "mov")
            {
                return "video/quicktime";
            }
            if (ext == "m4v")
            {
                return "video/x-m4v";
            }
            return {};
        }
    } // namespace detail

    std::string file_result::file_name() const
    {
        if (!file_name_.empty())
        {
            return file_name_;
        }
        if (!full_path_.empty())
        {
            return std::filesystem::path(full_path_).filename().string();
        }
        return {};
    }

    std::string file_result::content_type() const
    {
        if (!content_type_.empty())
        {
            return content_type_;
        }
        const std::string ext = std::filesystem::path(full_path_).extension().string();
        if (!ext.empty())
        {
            // Path::GetExtension returns ".png"; strip the leading dot for the map.
            const std::string content = detail::content_type_from_extension(std::string_view(ext).substr(1));
            if (!content.empty())
            {
                return content;
            }
        }
        return "application/octet-stream"; // FileBase.DefaultContentType
    }

    void file_result::open_read_async(read_callback on_complete) const
    {
        std::ifstream stream(full_path_, std::ios::binary);
        std::vector<std::byte> bytes;
        if (stream)
        {
            // Read the whole file via the streambuf char iterators, then widen each char to a byte.
            const std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            bytes.reserve(contents.size());
            for (const char c : contents)
            {
                bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
            }
        }
        on_complete(bytes);
    }
} // namespace maui::storage
