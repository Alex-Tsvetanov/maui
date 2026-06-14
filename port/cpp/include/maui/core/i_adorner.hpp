#pragma once
// maui::core::i_adorner  <=  Microsoft.Maui.IAdorner
//
// A window-overlay element that adorns (draws a border around) a view. Ported from
// src/Core/src/Core/IAdorner.cs. An i_window_overlay_element (so an overlay can host + draw it) that
// additionally exposes the underlying i_view it borders and its density. An abstract class
// (PROFILE §11 — runtime polymorphism through the overlay element list).

#include "maui/core/i_window_overlay_element.hpp"

namespace maui::core
{
    class i_view;

    class i_adorner : public i_window_overlay_element
    {
    public:
        // C# IAdorner.Density — the density override for the bordering visual.
        [[nodiscard]] virtual float density() const = 0;

        // C# IAdorner.VisualView — the underlying view that makes up the border. NON-owning.
        [[nodiscard]] virtual i_view* visual_view() const = 0;

    protected:
        i_adorner() = default;
        i_adorner(const i_adorner&) = default;
        i_adorner(i_adorner&&) = default;
        i_adorner& operator=(const i_adorner&) = default;
        i_adorner& operator=(i_adorner&&) = default;
    };
} // namespace maui::core
