#pragma once
// maui::controls::file_image_source  <=  Microsoft.Maui.Controls.FileImageSource
//
// A concrete image source backed by a file path. Ported from src/Controls/src/Core/FileImageSource.cs +
// the IFileImageSource contract: it owns a std::string path; is_empty() mirrors C#'s
// string.IsNullOrEmpty(File) and file() returns the stored path.
//
// This file source is loaded SYNCHRONOUSLY by image_handler::map_source (a file path is local + cheap);
// the uri / stream sources (uri_image_source.hpp / stream_image_source.hpp) load asynchronously through
// the image_source_loader. Font image sources remain deferred.
//
// image_source is a small factory namespace-struct: image_source::from_file/from_uri/from_stream mint a
// shared source. Ownership: the image control owns the returned shared_ptr; i_image::source() hands back
// a raw borrow.

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/uri_image_source.hpp" // uri_image_source::default_cache_validity
#include "maui/core/cancellation_token.hpp"
#include "maui/core/font.hpp" // from_font signature
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes (the stream-provider return type)
#include "maui/core/move_only_function.hpp"
#include "maui/graphics/color.hpp" // from_font signature

namespace maui::controls
{
    class file_image_source : public maui::core::i_file_image_source
    {
    public:
        explicit file_image_source(std::string file) : file_(std::move(file))
        {
        }

        // C# FileImageSource.IsEmpty => string.IsNullOrEmpty(File).
        [[nodiscard]] bool is_empty() const override
        {
            return file_.empty();
        }
        [[nodiscard]] std::string_view file() const override
        {
            return file_;
        }

    private:
        std::string file_;
    };

    // Factory entry point (mirrors C#'s ImageSource.FromFile / FromUri / FromStream). A free-standing
    // struct rather than free functions so every source kind joins the same `image_source::` surface.
    struct image_source
    {
        // The async bytes producer a stream source wraps (see stream_image_source::bytes_provider).
        using stream_provider =
            maui::core::move_only_function<maui::core::image_bytes(const maui::core::cancellation_token&)>;

        // Synchronous-loadable file source.
        [[nodiscard]] static std::shared_ptr<maui::core::i_image_source> from_file(std::string path);
        // URI source (async load via the loader). caching_enabled defaults to true and cache_validity to
        // one day, matching C# UriImageSource's CachingEnabled / CacheValidity property defaults.
        [[nodiscard]] static std::shared_ptr<maui::core::i_image_source> from_uri(
            std::string uri, bool caching_enabled = true,
            std::chrono::milliseconds cache_validity = uri_image_source::default_cache_validity);
        // Stream (bytes-provider) source (async load via the loader).
        [[nodiscard]] static std::shared_ptr<maui::core::i_image_source> from_stream(stream_provider provider);
        // Font glyph source (async rasterize via the loader). The glyph is drawn from `font` (family + size
        // + auto-scaling) and tinted `color`. Mirrors building a FontImageSource { Glyph, Font, Color }.
        [[nodiscard]] static std::shared_ptr<maui::core::i_image_source> from_font(std::string glyph,
                                                                                   maui::core::font font,
                                                                                   maui::graphics::color color);
    };
} // namespace maui::controls
