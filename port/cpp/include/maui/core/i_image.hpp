#pragma once
// maui::core::i_image  <=  Microsoft.Maui.IImage
//
// The virtual-view contract for an image view. Ported from src/Core/src/Core/IImage.cs
// (IImage : IView, IImageSourcePart). This is a MINIMAL cut: only aspect() (the scaling mode) is modeled.
//
// OUT OF SCOPE this cut (the whole async image SOURCE subsystem is deferred — file/URI/stream loaders,
// IImageSource / IImageSourceHandler, caching): IImageSourcePart.Source / IsAnimationPlaying /
// UpdateIsLoading, and IImage.IsOpaque. Documented here, not stubbed — the image control ships
// aspect-only.

#include "maui/core/aspect.hpp"
#include "maui/core/i_view.hpp"

namespace maui::core
{
    class i_image : public i_view
    {
    public:
        [[nodiscard]] virtual maui::core::aspect aspect() const = 0;
    };
} // namespace maui::core
