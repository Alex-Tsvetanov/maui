#pragma once
// maui::core::uri_image_source_service  <=  Microsoft.Maui.UriImageSourceService
//
// Loads an i_uri_image_source into a native image, with an in-memory cache. Ported from
// src/Core/src/ImageSources/UriImageSourceService/UriImageSourceService.iOS.cs (GetImageAsync over an
// IUriImageSource → DownloadAndCacheImageAsync → NSData → UIImage). DEVIATIONS: the cache is the loader's
// IN-MEMORY map (not C#'s on-disk cache), so this service does NOT cache by itself — caching is handled
// by the loader (it keys cached results by uri when caching_enabled()). The byte fetch reads a `file://`
// URI as a local path; a production HTTP(S) stack is deferred (apple reads NSURL/NSData for file/http(s);
// headless handles only `file://` and never hits the network).
//
// Partial-class split (PROFILE §5): declared here; load() defined per backend
// (headless mirrors kind="uri"+detail=uri with no native; apple reads the URL bytes → NSImage).

#include "maui/core/i_image_source_service.hpp"

namespace maui::core
{
    class i_image_source;
    class cancellation_token;

    class uri_image_source_service final : public i_image_source_service
    {
    public:
        // Resolve `source` as an i_uri_image_source and load its URI bytes. An empty source / failed
        // fetch delivers a `!loaded()` result. See the per-backend definition.
        void load(i_image_source& source, const cancellation_token& token, completion on_result) override;
    };
} // namespace maui::core
