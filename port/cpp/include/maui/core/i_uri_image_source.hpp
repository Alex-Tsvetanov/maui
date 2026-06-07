#pragma once
// maui::core::i_uri_image_source  <=  Microsoft.Maui.IUriImageSource
//
// The virtual-view contract for an image sourced from a URI. Ported from
// src/Core/src/ImageSources/IUriImageSource.cs (IUriImageSource : IImageSource { Uri Uri; TimeSpan
// CacheValidity; bool CachingEnabled }). Header-only abstract interface — lives in maui_core, no .cpp.
//
// SIMPLIFICATIONS vs C#:
//   * Uri is modeled as a std::string_view (a UTF-8 URI string) rather than a parsed System.Uri value
//     type. The async uri service treats a `file://` URI as a local path and reads the bytes; a
//     production HTTP(S) stack is deferred (the apple service uses NSURL/NSData, headless tests use
//     `file://` + stream sources and never hit the network).
//   * CacheValidity (the TimeSpan expiry) is DEFERRED — the loader's in-memory cache has no expiry this
//     cut (disk caching + CacheValidity are documented in image_source_loader.hpp). Only CachingEnabled
//     is modeled, gating whether a uri result is cached in / served from the in-memory cache.

#include <string_view>

#include "maui/core/i_image_source.hpp"

namespace maui::core
{
    // Inherits the virtual destructor + protected copy/move from i_image_source (the layered-interface
    // convention, like i_file_image_source), adding the Uri + CachingEnabled getters.
    class i_uri_image_source : public i_image_source
    {
    public:
        // The URI string to load (C# IUriImageSource.Uri, simplified to a UTF-8 string). The referent is
        // owned by the concrete source and stays valid for its lifetime.
        [[nodiscard]] virtual std::string_view uri() const = 0;

        // Whether a loaded uri result may be cached and served from cache (C# IUriImageSource.CachingEnabled).
        [[nodiscard]] virtual bool caching_enabled() const = 0;
    };
} // namespace maui::core
