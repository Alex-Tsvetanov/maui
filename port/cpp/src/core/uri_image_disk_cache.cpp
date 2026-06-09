// uri_image_disk_cache — the persistent byte cache under the loader's in-memory TTL cache. See the header.
// Ports the on-disk half of UriImageSourceService.iOS.cs (GetCachedFileName / CacheImage / GetCachedImage /
// IsImageCached) with the CacheValidity TTL applied via a sidecar timestamp + the injected clock.

#include "maui/core/uri_image_disk_cache.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "maui/core/crc64.hpp"
#include "maui/core/i_stream_image_source.hpp"

namespace maui::core
{
    namespace
    {
        // C#'s CacheDirectory tail: "com.microsoft.maui" / "MauiUriImages" under the platform cache root.
        constexpr std::string_view cache_subdir = "com.microsoft.maui";
        constexpr std::string_view images_subdir = "MauiUriImages";

        // The extension of `uri`'s path (C# Path.GetExtension(Uri.AbsolutePath)) — the substring from the
        // last '.' in the final path segment, or empty. Query/fragment are stripped first so "?v=2" tails
        // do not leak into the extension. Includes the leading '.', matching Path.GetExtension.
        std::string extension_of(std::string_view uri)
        {
            // Drop a query/fragment so they are not mistaken for part of the path.
            const std::size_t cut = uri.find_first_of("?#");
            const std::string_view path = cut == std::string_view::npos ? uri : uri.substr(0, cut);
            const std::size_t last_slash = path.find_last_of("/\\");
            const std::string_view leaf = last_slash == std::string_view::npos ? path : path.substr(last_slash + 1);
            const std::size_t dot = leaf.find_last_of('.');
            if (dot == std::string_view::npos)
            {
                return {};
            }
            return std::string(leaf.substr(dot));
        }

        // The MauiUriImages directory under the configured base.
        std::filesystem::path images_dir(const std::string& base)
        {
            return std::filesystem::path(base) / cache_subdir / images_subdir;
        }

        // Read every byte of `file` (binary) into an image_bytes (no reinterpret_cast — istreambuf_iterator
        // yields chars, converted to std::byte; a char* → std::byte* reinterpret would need a forbidden
        // NOLINT). Returns empty if the stream is not good.
        image_bytes read_all_bytes(std::ifstream& file)
        {
            image_bytes bytes;
            for (std::istreambuf_iterator<char> it(file), end; it != end; ++it)
            {
                bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(*it)));
            }
            return bytes;
        }
    } // namespace

    std::string uri_image_disk_cache::path_for(std::string_view uri) const
    {
        if (base_directory_.empty())
        {
            return {};
        }
        // C# GetCachedFileName: Crc64.ComputeHashString(uri) + Path.GetExtension(AbsolutePath).
        const std::string filename = crc64_hash_string(uri) + extension_of(uri);
        return (images_dir(base_directory_) / filename).string();
    }

    std::optional<uri_image_disk_cache::hit> uri_image_disk_cache::read(std::string_view uri,
                                                                        std::chrono::steady_clock::time_point now,
                                                                        std::chrono::milliseconds validity) const
    {
        if (base_directory_.empty())
        {
            return std::nullopt;
        }
        const std::filesystem::path data_path = path_for(uri);
        std::error_code ec;
        if (!std::filesystem::exists(data_path, ec) || ec)
        {
            return std::nullopt; // C# IsImageCached == false → a miss
        }

        // CacheValidity gate: the sidecar holds the steady-clock tick the bytes were written at. A missing or
        // unreadable sidecar, or an entry older than `validity`, is treated as a miss (re-fetch + re-stamp).
        const std::filesystem::path meta_path = data_path.string() + ".meta";
        std::ifstream meta(meta_path, std::ios::binary);
        if (!meta)
        {
            return std::nullopt;
        }
        std::int64_t stamped_ticks = 0;
        meta >> stamped_ticks;
        if (!meta)
        {
            return std::nullopt;
        }
        using rep = std::chrono::steady_clock::duration;
        const std::chrono::steady_clock::time_point cached_at{rep{stamped_ticks}};
        const std::chrono::steady_clock::duration age = now - cached_at;
        // Expired, OR a NEGATIVE age — the latter means the sidecar tick came from a different steady_clock
        // epoch (a prior process: steady_clock is per-process, often boot-relative). Rather than trust an
        // out-of-epoch stamp as "fresh forever", treat a negative age as a miss so the entry is re-fetched +
        // re-stamped against this process's clock. (Within a process the clock is monotonic, so age >= 0.)
        if (age < std::chrono::steady_clock::duration::zero() || age >= validity)
        {
            return std::nullopt;
        }

        std::ifstream data(data_path, std::ios::binary);
        if (!data)
        {
            return std::nullopt;
        }
        image_bytes bytes = read_all_bytes(data);
        if (bytes.empty())
        {
            return std::nullopt; // an empty file is no better than a miss
        }
        return hit{.bytes = std::move(bytes), .cached_at = cached_at};
    }

    bool uri_image_disk_cache::write(std::string_view uri, const image_bytes& bytes,
                                     std::chrono::steady_clock::time_point now) const
    {
        if (base_directory_.empty() || bytes.empty())
        {
            return false;
        }
        std::error_code ec;
        std::filesystem::create_directories(images_dir(base_directory_), ec); // C# Directory.CreateDirectory
        if (ec)
        {
            return false;
        }

        const std::filesystem::path data_path = path_for(uri);
        std::ofstream data(data_path, std::ios::binary | std::ios::trunc);
        if (!data)
        {
            return false;
        }
        for (const std::byte b : bytes)
        {
            data.put(static_cast<char>(std::to_integer<unsigned char>(b)));
        }
        data.flush();
        if (!data)
        {
            return false;
        }

        // Stamp the write time (steady-clock ticks) into the sidecar for the CacheValidity gate on read.
        const auto ticks = static_cast<std::int64_t>(now.time_since_epoch().count());
        std::ofstream meta(data_path.string() + ".meta", std::ios::binary | std::ios::trunc);
        if (!meta)
        {
            return false;
        }
        meta << ticks;
        meta.flush();
        return static_cast<bool>(meta);
    }
} // namespace maui::core
