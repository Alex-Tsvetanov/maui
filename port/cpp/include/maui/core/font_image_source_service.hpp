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
// handle; src/platform/{apple,ios}/image_source_services.mm draw the glyph into an NSImage/UIImage).
//
// The glyph's typeface is resolved through the FontManager (C#'s FontImageSourceService(IFontManager)):
// the per-backend load() resolves default_font_manager().get_font(source.font, DefaultFontSize), so a
// registered/aliased family renders in the right typeface (the registrar alias/embedded resolution +
// weight/slant traits + Dynamic Type scaling). The process-wide manager is the no-DI seam (PROFILE §6),
// matching how the loader resolves services against the process-wide registry.
//
// DEVIATIONS vs C#:
//   * The manager is resolved from the process-wide default_font_manager() rather than DI-injected into
//     the service ctor (C++23 has no reflection/DI container — PROFILE §6). A host configures app fonts in
//     default_font_registrar(); the manager reads them.
//   * The `scale` (display density) argument C#'s GetImageAsync takes is omitted here; the result is marked
//     resolution-dependent so the handler's scale seam re-issues the load on a density change (RequiresReload).

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
