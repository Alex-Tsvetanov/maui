#pragma once
// maui::controls::uri_image_source  <=  Microsoft.Maui.Controls.UriImageSource
//
// A concrete image source backed by a URI string + a caching flag. Ported from
// src/Controls/src/Core/UriImageSource.cs (the Uri / CachingEnabled bindable properties + IsEmpty =>
// Uri == null). It owns a std::string uri; is_empty() mirrors C#'s `Uri == null` as "the uri string is
// empty", caching_enabled() defaults to true (C# CachingEnabledProperty default).
//
// SIMPLIFICATION: the uri is a plain UTF-8 string (not a parsed System.Uri); CacheValidity (TimeSpan
// expiry) is not modeled — see i_uri_image_source.hpp. The bytes are fetched + decoded ASYNCHRONOUSLY
// by uri_image_source_service via the image_source_loader (the apple service reads NSURL/NSData; the
// headless service only handles `file://` and never hits the network).
//
// Minted via image_source::from_uri(uri) (the factory lives alongside from_file in file_image_source.hpp).

#include <string>
#include <string_view>
#include <utility>

#include "maui/core/i_uri_image_source.hpp"

namespace maui::controls
{
    class uri_image_source : public maui::core::i_uri_image_source
    {
    public:
        explicit uri_image_source(std::string uri, bool caching_enabled = true)
            : uri_(std::move(uri)), caching_enabled_(caching_enabled)
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
        [[nodiscard]] bool caching_enabled() const override
        {
            return caching_enabled_;
        }

    private:
        std::string uri_;
        bool caching_enabled_;
    };
} // namespace maui::controls
