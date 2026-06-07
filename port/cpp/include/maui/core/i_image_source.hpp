#pragma once
// maui::core::i_image_source  <=  Microsoft.Maui.IImageSource
// maui::core::i_file_image_source  <=  Microsoft.Maui.IFileImageSource
//
// The virtual-view contracts for an image's source. Ported from src/Core/src/ImageSources/IImageSource.cs
// (IImageSource { bool IsEmpty }) and IFileImageSource.cs (IFileImageSource : IImageSource { string File }).
// Header-only (abstract interfaces, no out-of-line state) — they live in maui_core with no .cpp.
//
// This is a FIRST CUT of the image-source subsystem: only the file source is modeled. The other concrete
// sources (uri / stream / font image sources) and the async loader / service-provider seam / caching are
// deferred — documented here, not stubbed. The image control loads a file source SYNCHRONOUSLY this cut.

#include <string_view>

namespace maui::core
{
    class i_image_source
    {
    public:
        virtual ~i_image_source() = default;

        // True when the source carries nothing to load (C# IImageSource.IsEmpty).
        [[nodiscard]] virtual bool is_empty() const = 0;

    protected:
        i_image_source() = default;
        i_image_source(const i_image_source&) = default;
        i_image_source(i_image_source&&) = default;
        i_image_source& operator=(const i_image_source&) = default;
        i_image_source& operator=(i_image_source&&) = default;
    };

    // A derived interface: it inherits the virtual destructor + protected copy/move from i_image_source
    // (matching the codebase convention for layered interfaces like i_text_input : i_text), adding only the
    // File getter.
    class i_file_image_source : public i_image_source
    {
    public:
        // The file path (or bundle resource name) to load (C# IFileImageSource.File). The referent is owned
        // by the concrete source and stays valid for its lifetime.
        [[nodiscard]] virtual std::string_view file() const = 0;
    };
} // namespace maui::core
