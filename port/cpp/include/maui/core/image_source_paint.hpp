#pragma once
// maui::core::image_source_paint  <=  Microsoft.Maui.ImageSourcePaint
//
// A Paint subclass whose fill is an image source — the brush behind an image-backed view Background.
// Ported from src/Core/src/ImageSources/ImageSourcePaint.cs (a Paint carrying an IImageSource). When a
// view's Background is an image_source_paint, the platform background mapper resolves the source through
// the image service provider, loads its native image, and tiles/stretches it as the view's backing layer
// (the iOS PageExtensions.UpdateBackground / ViewExtensions.UpdateBackgroundImageSourceAsync branch).
//
// Inherits maui::graphics::paint (the abstract brush base, PROFILE §11 — runtime-polymorphic via the
// view_mapper). background_color() reports transparent and is_transparent() true: an image paint has no
// flat fill color, and the view's solid-color background is cleared while the image layer is installed
// (C# sets BackgroundColor = UIColor.Clear). The image source is held NON-OWNING (a raw borrow): the
// control owns the source's lifetime (the same i_image_source the view's image stack would own); the paint
// just references which source to render, mirroring C#'s ImageSourcePaint.ImageSource property.
//
// A sibling unit (X1 Brushes / ImageBrush) builds on this; the rendering primitive (the per-backend
// apply_image_source_background) is kept clean + reusable for that.

#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::core
{
    class i_image_source;

    class image_source_paint final : public maui::graphics::paint
    {
    public:
        image_source_paint() = default;
        explicit image_source_paint(i_image_source* source) : source_(source)
        {
        }

        // C# ImageSourcePaint.ImageSource — the source to render as the fill (non-owning borrow; null when
        // unset). The control owns the source's lifetime.
        [[nodiscard]] i_image_source* image_source() const noexcept
        {
            return source_;
        }
        void set_image_source(i_image_source* source) noexcept
        {
            source_ = source;
        }

        // An image paint has no flat color: report a fully transparent color (the solid background is
        // cleared while the image layer is shown — C# UIColor.Clear). NOTE: the default color() is opaque
        // black, so transparency is set explicitly. The view_mapper's solid-fill branch is bypassed for an
        // image paint; the per-backend mapper detects this type and renders the image instead.
        [[nodiscard]] maui::graphics::color background_color() const override
        {
            return maui::graphics::color{0.0F, 0.0F, 0.0F, 0.0F}; // transparent (alpha 0)
        }
        [[nodiscard]] bool is_transparent() const override
        {
            return true;
        }

    private:
        i_image_source* source_ = nullptr; // non-owning (the control owns the source)
    };
} // namespace maui::core
