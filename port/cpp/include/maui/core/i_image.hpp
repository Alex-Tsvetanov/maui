#pragma once
// maui::core::i_image  <=  Microsoft.Maui.IImage
//
// The virtual-view contract for an image view. Ported from src/Core/src/Core/IImage.cs
// (IImage : IView, IImageSourcePart). This cut models aspect() (the scaling mode) and source() (the
// IImageSourcePart.Source — a FIRST CUT loading file sources synchronously).
//
// OUT OF SCOPE this cut (deferred, documented not stubbed): IsAnimationPlaying / UpdateIsLoading and
// IImage.IsOpaque; and within the source subsystem, the non-file sources (uri/stream/font) plus async
// loading + cancellation + caching.

#include "maui/core/aspect.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_image : public i_view
    {
    public:
        [[nodiscard]] virtual maui::core::aspect aspect() const = 0;

        // The image's source (C# IImageSourcePart.Source). A raw borrow: the concrete control owns the
        // source (a shared_ptr) for its lifetime; null means "no source". Synchronous file load this cut.
        [[nodiscard]] virtual maui::core::i_image_source* source() const = 0;
    };
} // namespace maui::core
