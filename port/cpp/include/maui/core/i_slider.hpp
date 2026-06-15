#pragma once
// maui::core::i_slider  <=  Microsoft.Maui.ISlider
//
// The virtual-view contract for a linear-value input: an IView + IRange with the track/thumb colors
// and the drag notifications. Ported from src/Core/src/Core/ISlider.cs (ISlider : IView, IRange).
//
// Naming note (the i_button convention): C#'s ISlider.DragStarted()/DragCompleted() are spelled
// send_drag_started/send_drag_completed — the inbound channel the platform view calls (matching C#'s
// ISliderController.SendDragStarted trigger semantics), leaving `drag_started`/`drag_completed` free
// for the control's developer-facing events.
//
// ThumbImageSource IS ported: thumb_image_source() returns the optional image source the handler fetches
// through the shared IImageSourceService seam (the async image-service load + native thumb swap), exactly
// like ISlider.ThumbImageSource. A null source = no thumb image (the ThumbColor branch applies instead).

#include "maui/core/i_image_source.hpp"
#include "maui/core/i_range.hpp"
#include "maui/core/i_view.hpp"
#include "maui/graphics/color.hpp"

namespace maui::core
{
    class i_slider : public i_view, public i_range
    {
    public:
        // The color of the track portion holding the minimum value (the filled side).
        [[nodiscard]] virtual maui::graphics::color minimum_track_color() const = 0;
        // The color of the track portion holding the maximum value (the unfilled side).
        [[nodiscard]] virtual maui::graphics::color maximum_track_color() const = 0;
        // The color of the draggable thumb.
        [[nodiscard]] virtual maui::graphics::color thumb_color() const = 0;

        // C# ISlider.ThumbImageSource — the image drawn on the thumb (null = none; the handler resolves it
        // through the IImageSourceService and swaps the native thumb image). A NON-owning borrow (the
        // control owns the source via shared_ptr), matching i_image::source().
        [[nodiscard]] virtual maui::core::i_image_source* thumb_image_source() const = 0;

        // Inbound channel (called by the platform view when the user starts/finishes dragging).
        virtual void send_drag_started() = 0;
        virtual void send_drag_completed() = 0;
    };
} // namespace maui::core
