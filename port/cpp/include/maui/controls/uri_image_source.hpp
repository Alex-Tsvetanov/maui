#pragma once
// maui::controls::uri_image_source  <=  Microsoft.Maui.Controls.UriImageSource
//
// A concrete image source backed by a URI string + a cache-validity window + a caching flag. Ported from
// src/Controls/src/Core/UriImageSource.cs (the Uri / CacheValidity / CachingEnabled bindable properties +
// IsEmpty => Uri == null). It owns a std::string uri; is_empty() mirrors C#'s `Uri == null` as "the uri
// string is empty", cache_validity() defaults to one day (C# CacheValidityProperty's TimeSpan.FromDays(1)),
// caching_enabled() defaults to true (C# CachingEnabledProperty default).
//
// SIMPLIFICATION: the uri is a plain UTF-8 string (not a parsed System.Uri). CacheValidity is a
// std::chrono::milliseconds the loader honors as a TTL — see i_uri_image_source.hpp. The bytes are
// fetched + decoded ASYNCHRONOUSLY by uri_image_source_service via the image_source_loader (the apple
// service reads NSURL/NSData; the headless service only handles `file://` and never hits the network).
//
// Minted via image_source::from_uri(uri) (the factory lives alongside from_file in file_image_source.hpp).

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/i_uri_image_source.hpp"

namespace maui::controls
{
    class uri_image_source : public maui::core::i_uri_image_source
    {
    public:
        // C# CacheValidityProperty default: TimeSpan.FromDays(1).
        static constexpr std::chrono::milliseconds default_cache_validity = std::chrono::hours(24);

        explicit uri_image_source(std::string uri, bool caching_enabled = true,
                                  std::chrono::milliseconds cache_validity = default_cache_validity)
            : uri_(std::move(uri)), cache_validity_(cache_validity), caching_enabled_(caching_enabled)
        {
        }

        // C# UriImageSource.IsEmpty => Uri == null, modeled as "the uri string is empty".
        [[nodiscard]] bool is_empty() const override
        {
            return uri_.empty();
        }
        [[nodiscard]] std::string_view uri() const override
        {
            return uri_;
        }
        [[nodiscard]] std::chrono::milliseconds cache_validity() const override
        {
            return cache_validity_;
        }
        [[nodiscard]] bool caching_enabled() const override
        {
            return caching_enabled_;
        }

    private:
        std::string uri_;
        std::chrono::milliseconds cache_validity_;
        bool caching_enabled_;
    };
} // namespace maui::controls
