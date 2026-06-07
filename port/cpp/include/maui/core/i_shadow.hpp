#pragma once
// maui::core::i_shadow  <=  Microsoft.Maui.IShadow
//
// The drop-shadow contract a view exposes via IView.Shadow. Ported from src/Core/src/Core/IShadow.cs:
// Radius (blur), Opacity, Paint (the colorizing brush — only SolidPaint is honored on all platforms),
// and Offset. An abstract class (PROFILE §11 — runtime polymorphism: the view_mapper pushes an i_shadow*
// through the platform base).
//
// Ownership: the control owns the shadow via a shared_ptr (controls/view.hpp); i_view::shadow() returns a
// raw borrow and the view_platform_base mirror is a non-owning const pointer. The shadow itself owns its
// paint; i_shadow::paint() likewise hands back a raw borrow.

#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"

namespace maui::core
{
    class i_shadow
    {
    public:
        virtual ~i_shadow() = default;

        // C# IShadow.Radius — the blur radius used to generate the shadow.
        [[nodiscard]] virtual double radius() const = 0;
        // C# IShadow.Opacity — the shadow's opacity.
        [[nodiscard]] virtual double opacity() const = 0;
        // C# IShadow.Paint — the brush colorizing the shadow (a borrow owned by the shadow; may be null).
        [[nodiscard]] virtual maui::graphics::paint* paint() const = 0;
        // C# IShadow.Offset — the shadow's offset relative to the view.
        [[nodiscard]] virtual maui::graphics::point offset() const = 0;

    protected:
        i_shadow() = default;
        i_shadow(const i_shadow&) = default;
        i_shadow(i_shadow&&) = default;
        i_shadow& operator=(const i_shadow&) = default;
        i_shadow& operator=(i_shadow&&) = default;
    };
} // namespace maui::core
