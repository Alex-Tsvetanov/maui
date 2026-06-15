#pragma once
// maui::graphics::i_graphics_image  <=  Microsoft.Maui.Graphics.IImage
//
// The drawing-layer image abstraction: an image that can draw itself onto a canvas (IDrawable) and
// expose its pixel dimensions plus a platform-specific representation for the canvas to blit. Ported
// from src/Graphics/src/Graphics/IImage.cs.
//
// SCOPE (recorded in port/STATUS.md): only the slice the canvas drawing surface needs —
// width/height + to_platform_image() (the handle ICanvas.DrawImage hands to the backend). C#'s
// Downsize/Resize/Save/SaveAsync and the ResizeMode enum are documented-deferred (they land with a
// real image-loading/codec consumer); IDisposable becomes RAII (the derived type owns its handle).
//
// Deliberate deviations:
//  - C#'s IImage : IDisposable; teardown is the C++ destructor (PROFILE §8). The interface keeps the
//    virtual destructor i_drawable already supplies.
//  - ToPlatformImage() returns an IImage in C#; here to_platform_image() returns an opaque void*
//    handle so the drawing layer stays free of any platform image type — the CoreGraphics backend
//    casts it to a CGImageRef, headless returns nullptr. (The C# round-trip-to-self contract is not
//    modeled; the canvas only needs the platform handle.) The handle is non-const void* (not const
//    void*): CoreGraphics' CGContextDrawImage takes a mutable CGImageRef, and the handle is the
//    pointer value itself, so a const here would only force a const_cast at the blit with no real
//    safety. Ownership stays with the image; the canvas reads the handle, never frees it.

#include "maui/graphics/i_drawable.hpp"

namespace maui::graphics
{
    class i_graphics_image : public i_drawable
    {
    public:
        // C# IImage.Width — the image width in pixels.
        [[nodiscard]] virtual float width() const = 0;
        // C# IImage.Height — the image height in pixels.
        [[nodiscard]] virtual float height() const = 0;

        // C# IImage.ToPlatformImage() — the platform-specific representation the canvas blits. The
        // backend knows the concrete type (CoreGraphics: CGImageRef); the drawing layer treats it as
        // opaque. May be null when the image has no platform representation (e.g. the headless test
        // image). The handle is owned by this image; the canvas does not retain it past the draw.
        [[nodiscard]] virtual void* to_platform_image() const = 0;

        // C# IImage.Downsize/Resize/Save/SaveAsync are deferred — see the header note.

    protected:
        i_graphics_image() = default;
        i_graphics_image(const i_graphics_image&) = default;
        i_graphics_image(i_graphics_image&&) = default;
        i_graphics_image& operator=(const i_graphics_image&) = default;
        i_graphics_image& operator=(i_graphics_image&&) = default;
    };
} // namespace maui::graphics
