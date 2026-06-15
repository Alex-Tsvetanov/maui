#pragma once
// maui::graphics::headless_image  <=  (test/headless stand-in for Microsoft.Maui.Graphics.PlatformImage)
//
// A backend-free i_graphics_image for the headless backend and the canvas op tests: it carries width
// and height but has no platform representation (to_platform_image() returns null), so the recording
// canvas records draw_image geometry without any image handle.
//
// draw() mirrors the C# PlatformImage.Draw across all platforms
// (src/Graphics/src/Graphics/Platforms/iOS/PlatformImage.cs:156 et al.):
//   canvas.DrawImage(this, dirtyRect.Left, dirtyRect.Top, MathF.Round(Width), MathF.Round(Height)).
// Header-only — there is no headless platform-image codec to implement (Downsize/Resize/Save are
// the deferred slice on i_graphics_image, and this stub does not add them either).

#include <cmath>

#include "maui/graphics/i_canvas.hpp"
#include "maui/graphics/i_graphics_image.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::graphics
{
    class headless_image final : public i_graphics_image
    {
    public:
        headless_image(float width, float height) : width_(width), height_(height)
        {
        }

        [[nodiscard]] float width() const override
        {
            return width_;
        }

        [[nodiscard]] float height() const override
        {
            return height_;
        }

        // No platform representation — the headless canvas records geometry only.
        [[nodiscard]] void* to_platform_image() const override
        {
            return nullptr;
        }

        // C# PlatformImage.Draw — blit self into the dirty rect via the canvas DrawImage contract.
        void draw(i_canvas& canvas, const rect_f& dirty_rect) override
        {
            canvas.draw_image(*this, dirty_rect.left(), dirty_rect.top(), std::round(dirty_rect.width),
                              std::round(dirty_rect.height));
        }

    private:
        float width_;
        float height_;
    };
} // namespace maui::graphics
