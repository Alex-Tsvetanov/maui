#pragma once
// maui::core::uri_image_disk_cache  <=  Microsoft.Maui.UriImageSourceService.DownloadAndCacheImageAsync
//                                       (the on-disk caching half) + GetCachedFileName / CacheImage /
//                                       GetCachedImage / IsImageCached (UriImageSourceService.iOS.cs)
//
// A persistent, cross-platform byte cache for fetched URI image data, layered UNDER the loader's in-memory
// TTL cache. Ported from the iOS UriImageSourceService disk cache:
//   * the cache directory is "<base>/com.microsoft.maui/MauiUriImages" (C# CacheDirectory; the apple
//     handler seeds <base> from NSCachesDirectory, headless from a temp dir — see set_directory).
//   * the file name is crc64(uri) + the uri's extension (C# GetCachedFileName: Crc64.ComputeHashString +
//     Path.GetExtension(AbsolutePath)).
//   * a cache HIT short-circuits the network fetch (C# `if (CachingEnabled && IsImageCached) GetCachedImage`).
//   * a fetched payload is written back (C# CacheImage → Directory.CreateDirectory + NSData.Save).
//
// TTL: C#'s disk layer is a bare File.Exists (no expiry on disk — CacheValidity governs only the in-memory
// HttpClient cache there). This port instead HONORS CacheValidity on the disk entry too, via the SAME
// injected clock seam the in-memory cache uses (image_source_loader::set_clock): a sidecar ".meta" file
// records the steady-clock tick the bytes were written at, and an entry older than `validity` is treated as
// a miss + re-fetched. DEVIATION (documented): the timestamp is a steady_clock tick, monotonic within a
// process; across a process restart the in-memory cache is empty anyway, so a re-validation / re-fetch on
// the first post-restart load is acceptable and matches the in-memory cache's lifetime. No wall-clock
// dependency → the headless tests drive expiry deterministically through the clock seam.
//
// Threading (PROFILE §8): pure std::filesystem I/O, no internal threads or shared mutable state. The loader
// calls read()/write() ON THE UI THREAD (the fast-path read before dispatching a fetch; the write inside
// the dispatched completion). The only cross-thread elements in the image stack stay the cancellation
// atomic + the dispatcher hand-off — this class adds none.

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/i_stream_image_source.hpp" // image_bytes

namespace maui::core
{
    class uri_image_disk_cache
    {
    public:
        uri_image_disk_cache() = default;

        // The base directory under which "com.microsoft.maui/MauiUriImages" is created (C# CacheDirectory's
        // FileSystem.CacheDirectory root). The apple handler passes NSCachesDirectory; headless tests pass a
        // unique temp dir. An empty base disables the cache (read/write become no-ops). Non-owning copy.
        void set_directory(std::string base_directory)
        {
            base_directory_ = std::move_if_noexcept(base_directory);
        }

        // True when a cache directory is configured (the cache is active).
        [[nodiscard]] bool enabled() const
        {
            return !base_directory_.empty();
        }

        // The absolute path the bytes for `uri` are cached at (crc64(uri)+ext under MauiUriImages). Empty
        // when the cache is disabled. Exposed for tests/inspection (C# GetCachedFileName + Path.Combine).
        [[nodiscard]] std::string path_for(std::string_view uri) const;

        // A successful read: the cached payload + the ORIGINAL write time recorded in the sidecar (so the
        // caller can repopulate its in-memory layer measuring age from the original fetch, not the read).
        struct hit
        {
            image_bytes bytes;
            std::chrono::steady_clock::time_point cached_at;
        };

        // Read the cached bytes for `uri` if a FRESH entry exists: the payload file is present AND its
        // recorded write time is within `validity` of `now` (both from the injected clock). Returns
        // std::nullopt on a miss (no entry, expired, or cache disabled). C# IsImageCached + GetCachedImage,
        // plus the CacheValidity gate.
        [[nodiscard]] std::optional<hit> read(std::string_view uri, std::chrono::steady_clock::time_point now,
                                              std::chrono::milliseconds validity) const;

        // Persist `bytes` for `uri`, stamping the write time `now` (the injected clock) into the sidecar.
        // Creates the cache directory if missing (C# CacheImage). A no-op when the cache is disabled or the
        // bytes are empty. Returns true on a successful write.
        bool write(std::string_view uri, const image_bytes& bytes, std::chrono::steady_clock::time_point now) const;

    private:
        std::string base_directory_;
    };
} // namespace maui::core
