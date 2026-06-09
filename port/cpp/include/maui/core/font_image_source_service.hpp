#pragma once
// maui::core::font_image_source_service  <=  Microsoft.Maui.FontImageSourceService
//
// Rasterizes an i_font_image_source (a glyph + font + color) into a native image. Ported from
// src/Core/src/ImageSources/FontImageSourceService/FontImageSourceService.cs (the abstract base) +
// FontImageSourceService.iOS.cs (GetImageAsync over an IFontImageSource → UIImage via GetPlatformImage,
// which draws the glyph in the font/color into a UIImage). The empty/wrong-type source delivers a
// `!loaded()` result.
//
// Partial-class split (PROFILE §5): declared here; load() defined per backend
// (src/platform/headless/image_source_services.cpp mirrors kind="font"+detail=the glyph with no native
// handle; src/platform/apple/image_source_services.mm draws the glyph into an NSImage).
//
// DEVIATIONS vs C#:
//   * C#'s FontImageSourceService is constructed with an IFontManager that resolves a registered font by
//     family name. The port has no font manager, so the service renders with the source's font value
//     directly (apple converts font→NSFont via apple_conversions; an unresolved family falls back to the
//     system font, exactly as to_ns_font already does). Documented, not stubbed.
//   * The `scale` (display density) argument C#'s GetImageAsync takes is omitted (resolution-dependent
//     reload is handled by the handler's scale seam, not the service).

#include "maui/core/i_image_source_service.hpp"

namespace maui::core
{
    class i_image_source;
    class cancellation_token;

    class font_image_source_service final : public i_image_source_service
    {
    public:
        // Resolve `source` as an i_font_image_source and rasterize its glyph. An empty source / wrong type
        // delivers a `!loaded()` result. See the per-backend definition.
        void load(i_image_source& source, const cancellation_token& token, completion on_result) override;
    };
} // namespace maui::core
