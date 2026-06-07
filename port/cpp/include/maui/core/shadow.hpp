#pragma once
// maui::core::shadow  <=  Microsoft.Maui.Controls.Shadow (the concrete IShadow)
//
// A concrete drop shadow. Ported from src/Controls/src/Core/Shadow.cs: the bindable defaults are
// Radius = 10, Opacity = 1, Brush = Black, Offset = null (which resolves to the default Point {0,0}).
// Here it is a plain value type holding those fields (the binding layer is M5); it OWNS its paint (via a
// shared_ptr, the codebase's borrow idiom — cf. image::source) so i_shadow::paint() hands back a stable
// raw borrow without a const_cast.
//
// The control owns a shadow via a shared_ptr (controls/view.hpp); i_view::shadow() returns a raw borrow.
// The out-of-line definitions live in shadow.cpp (one primary type per header + matching .cpp).

#include <memory>

#include "maui/core/i_shadow.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/point.hpp"

namespace maui::core
{
    class shadow : public i_shadow
    {
    public:
        // Defaults mirror Shadow.cs: radius 10, opacity 1, black paint, zero offset. The black paint is
        // materialized in the constructor (defined out-of-line in shadow.cpp).
        shadow();

        // ---- i_shadow ----
        [[nodiscard]] double radius() const override;
        [[nodiscard]] double opacity() const override;
        // The owned paint, as a borrow (never null for a default shadow — it is the black solid_paint).
        [[nodiscard]] maui::graphics::paint* paint() const override;
        [[nodiscard]] maui::graphics::point offset() const override;

        // ---- setters (the binding layer is M5; these are the value-type mutators) ----
        void set_radius(double value);
        void set_opacity(double value);
        // Replaces the colorizing brush with a solid paint of the given color.
        void set_color(maui::graphics::color value);
        void set_offset(maui::graphics::point value);

    private:
        double radius_ = 10.0;
        double opacity_ = 1.0;
        maui::graphics::point offset_;
        // Brush.Black default (an opaque-black solid paint); materialized in the constructor body.
        std::shared_ptr<maui::graphics::paint> paint_;
    };
} // namespace maui::core
