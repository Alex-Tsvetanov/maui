#pragma once
// maui::core::stream_image_source_service  <=  Microsoft.Maui.StreamImageSourceService
//
// Loads an i_stream_image_source (a bytes provider) into a native image. Ported from
// src/Core/src/ImageSources/StreamImageSourceService/StreamImageSourceService.iOS.cs (GetImageAsync over
// an IStreamImageSource → GetStreamAsync → CGImageSource → UIImage). Here the source yields the encoded
// bytes directly (i_stream_image_source::get_bytes), which the service decodes to a native image.
//
// Partial-class split (PROFILE §5): declared here; load() defined per backend
// (headless mirrors kind="stream"+detail="<bytes:N>" with no native; apple decodes NSData → NSImage).

#include "maui/core/i_image_source_service.hpp"

namespace maui::core
{
    class i_image_source;
    class cancellation_token;

    class stream_image_source_service final : public i_image_source_service
    {
    public:
        // Resolve `source` as an i_stream_image_source, pull its bytes (honoring `token`), and decode
        // them. Empty bytes / a failed decode delivers a `!loaded()` result. See the per-backend definition.
        void load(i_image_source& source, const cancellation_token& token, completion on_result) override;
    };
} // namespace maui::core
