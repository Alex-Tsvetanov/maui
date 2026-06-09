#pragma once
// maui::core::i_image  <=  Microsoft.Maui.IImage  (: IView, IImageSourcePart)
//
// The virtual-view contract for an image view. Ported from src/Core/src/Core/IImage.cs:
//   IImage             : IView, IImageSourcePart { Aspect Aspect; bool IsOpaque }
//   IImageSourcePart   { IImageSource? Source; bool IsAnimationPlaying; void UpdateIsLoading(bool) }
// This models the full IImage + IImageSourcePart surface: aspect (scaling), source (the loaded source),
// is_opaque, is_animation_playing, and update_is_loading (the handler pushes the loader's loading state
// onto the view — C#'s IsLoading is a read-only property set through this).
//
// DEVIATION: GIF animation detection (IsAnimationPlaying's native interplay — UIImageView.AnimationImages)
// is NOT implemented; the property is faithful (settable + mapped) but the native backend only stores the
// flag (no multi-frame decode). Documented on image_handler / image.hpp.

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
        // source (a shared_ptr) for its lifetime; null means "no source".
        [[nodiscard]] virtual maui::core::i_image_source* source() const = 0;

        // C# IImage.IsOpaque — a hint that the renderer may skip drawing content behind the image.
        [[nodiscard]] virtual bool is_opaque() const = 0;

        // C# IImageSourcePart.IsAnimationPlaying — whether an animated (GIF) image is playing. The native
        // multi-frame animation is a documented deviation; the flag itself is faithful + mapped.
        [[nodiscard]] virtual bool is_animation_playing() const = 0;

        // C# IImageSourcePart.UpdateIsLoading — the handler/loader pushes the in-flight loading state onto
        // the view (the source of the control's read-only IsLoading). Non-const: it mutates view state.
        virtual void update_is_loading(bool is_loading) = 0;
    };
} // namespace maui::core
