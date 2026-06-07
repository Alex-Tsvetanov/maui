#pragma once
// maui::core::file_image_source_service  <=  Microsoft.Maui.FileImageSourceService
//
// Loads an i_file_image_source into a native image. Ported from
// src/Core/src/ImageSources/FileImageSourceService/FileImageSourceService.iOS.cs (GetImageAsync over an
// IFileImageSource → UIImage). The load is synchronous (a local file decodes cheaply); the result is
// delivered through the completion callback like the async services so the loader treats every kind
// uniformly.
//
// Partial-class split (PROFILE §5): the class is declared here; load() is defined per backend
// (src/platform/headless/image_source_services.cpp mirrors kind="file"+detail=path with no native handle;
// src/platform/apple/image_source_services.mm loads the file via NSImage into the result).

#include "maui/core/i_image_source_service.hpp"

namespace maui::core
{
    class i_image_source;
    class cancellation_token;

    class file_image_source_service final : public i_image_source_service
    {
    public:
        // Resolve `source` as an i_file_image_source and load its file. An empty source / failed load
        // delivers a `!loaded()` result. See the per-backend definition.
        void load(i_image_source& source, const cancellation_token& token, completion on_result) override;
    };
} // namespace maui::core
