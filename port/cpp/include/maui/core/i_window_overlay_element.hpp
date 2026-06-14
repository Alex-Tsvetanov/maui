#pragma once
// maui::core::i_window_overlay_element  <=  Microsoft.Maui.IWindowOverlayElement
//
// A drawable element drawn on top of an i_window_overlay. Ported from
// src/Core/src/Core/IWindowOverlayElement.cs. An i_drawable (so the overlay replays each element's
// Draw onto the shared canvas) plus a point hit-test (Contains) the overlay consults for
// drawable-touch handling. An abstract class (PROFILE §11 — the overlay invokes it polymorphically).

#include "maui/graphics/i_drawable.hpp"
#include "maui/graphics/point.hpp"

namespace maui::core
{
    class i_window_overlay_element : public maui::graphics::i_drawable
    {
    public:
        // C# IWindowOverlayElement.Contains(Point) — is `point` inside this element?
        [[nodiscard]] virtual bool contains(const maui::graphics::point& point) const = 0;

    protected:
        i_window_overlay_element() = default;
        i_window_overlay_element(const i_window_overlay_element&) = default;
        i_window_overlay_element(i_window_overlay_element&&) = default;
        i_window_overlay_element& operator=(const i_window_overlay_element&) = default;
        i_window_overlay_element& operator=(i_window_overlay_element&&) = default;
    };
} // namespace maui::core
