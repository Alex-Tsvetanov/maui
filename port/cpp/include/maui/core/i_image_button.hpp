#pragma once
// maui::core::i_image_button  <=  Microsoft.Maui.IImageButton
//
// The virtual-view contract for a button that displays an image. Ported from
// src/Core/src/Core/IImageButton.cs (IImageButton : IButton, IImage, IButtonStroke).
//
// C# composes the contract from IButton AND IImage, both of which extend IView. C++ cannot inherit
// both without a diamond over i_view (the port deliberately avoids virtual bases — see i_text_button's
// "no diamond" note), so this interface derives i_button (i_view + i_padding + i_button_stroke arrive
// through it) and RE-DECLARES the IImage/IImageSourcePart surface directly (documented deviation:
// the members are identical to i_image's — aspect/source/is_opaque/is_animation_playing/
// update_is_loading — but i_image_button is not an i_image subtype).

#include "maui/core/aspect.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::core
{
    class i_image_button : public i_button
    {
    public:
        // ---- the IImage / IImageSourcePart surface (mirrors i_image member-for-member) ----
        [[nodiscard]] virtual maui::core::aspect aspect() const = 0;
        // The image's source (C# IImageSourcePart.Source). A raw borrow: the concrete control owns the
        // source (a shared_ptr) for its lifetime; null means "no source".
        [[nodiscard]] virtual maui::core::i_image_source* source() const = 0;
        [[nodiscard]] virtual bool is_opaque() const = 0;
        // C# ImageButton pins IImageSourcePart.IsAnimationPlaying to false (no bindable behind it).
        [[nodiscard]] virtual bool is_animation_playing() const = 0;
        // The handler/loader pushes the in-flight loading state onto the view (drives IsLoading).
        virtual void update_is_loading(bool is_loading) = 0;
    };
} // namespace maui::core
