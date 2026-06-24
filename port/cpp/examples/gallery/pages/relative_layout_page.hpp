#pragma once
// maui::samples::relative_layout_page — ports RelativeLayoutPage.xaml.
//
// note: RelativeLayout is NOT ported. The C# sample uses
// Microsoft.Maui.Controls.Compatibility.RelativeLayout with ConstraintExpression markup (Type=Constant /
// RelativeToParent / RelativeToView). The whole Compatibility namespace — the legacy layout/renderer
// stack — is explicitly OUT OF SCOPE for this port (port/CLAUDE.md "Skip the out-of-scope namespaces:
// Compatibility…"), so there is no maui::controls::relative_layout to target. Per the porting rules we do
// NOT invent one; instead this page reproduces the SAME visual intent with the modern replacement MAUI
// itself recommends for RelativeLayout — absolute_layout with proportional LayoutFlags — and notes each
// place where the legacy constraint model has no absolute_layout equivalent.
//
// Constraint -> absolute_layout mapping (faithful where the model allows):
//   - "Constant=0" corner anchors  -> proportional X/Y of 0 (left/top) or 1 (right/bottom);
//   - "RelativeToParent Property=Width/Height, Factor=f"  -> proportional position/size of f;
//   - "RelativeToView ElementName=… "  -> HAS NO absolute_layout analog (absolute_layout positions a
//     child against the PARENT, never against a sibling). The C# black box (1/3 of the silver box,
//     positioned at the silver box's origin) is approximated as a small proportional box over the same
//     region, with the divergence noted inline.
//
// The page OWNS its whole element tree (the value_controls_page pattern). It is backend-agnostic — a
// sample main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the
// headless/apple/ios test trees exercise the same controls directly.
//
// Demonstrated:
//   - absolute_layout with proportional position + size flags as the RelativeLayout stand-in;
//   - four colored corner boxes + a centered 1/3-size box (the legacy constraint scene, modernized).

#include "maui/controls/absolute_layout.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/layouts/absolute_layout_flags.hpp"

namespace maui::samples
{
    class relative_layout_page
    {
    public:
        relative_layout_page()
        {
            page_.set_title("RelativeLayout");

            // The four corner boxes: 40x40 absolute size, position proportional so each pins to a corner.
            // (C# default BoxView measure is ~40x40; the legacy "-40" constants kept boxes flush to the
            // far edge — proportional X/Y of 1 does the same against the parent's far edge.)
            constexpr double box_side = 40.0;

            // <BoxView Color="Red"  X=Constant 0, Y=Constant 0> — top-left.
            red_.set_color(maui::graphics::color(0.85F, 0.20F, 0.18F));
            place(red_, 0.0, 0.0, box_side, box_side, maui::layouts::absolute_layout_flags::position_proportional);

            // <BoxView Color="Green" X=Width-40, Y=Constant 0> — top-right.
            green_.set_color(maui::graphics::color(0.18F, 0.65F, 0.30F));
            place(green_, 1.0, 0.0, box_side, box_side, maui::layouts::absolute_layout_flags::position_proportional);

            // <BoxView Color="Blue"  X=Constant 0, Y=Height-40> — bottom-left.
            blue_.set_color(maui::graphics::color(0.20F, 0.40F, 0.80F));
            place(blue_, 0.0, 1.0, box_side, box_side, maui::layouts::absolute_layout_flags::position_proportional);

            // <BoxView Color="Yellow" X=Width-40, Y=Height-40> — bottom-right.
            yellow_.set_color(maui::graphics::color(0.95F, 0.80F, 0.20F));
            place(yellow_, 1.0, 1.0, box_side, box_side, maui::layouts::absolute_layout_flags::position_proportional);

            // <BoxView x:Name="oneThird" Color="Silver" — centered, 1/3 width AND height of the parent.>
            // C# anchors its top-left at (Width*0.33, Height*0.33) and sizes it 0.33 x 0.33 — fully
            // proportional position + size.
            one_third_.set_color(maui::graphics::color(0.75F, 0.75F, 0.75F));
            place(one_third_, 0.33, 0.33, 0.33, 0.33,
                  maui::layouts::absolute_layout_flags::position_proportional |
                      maui::layouts::absolute_layout_flags::size_proportional);

            // <BoxView Color="Black"> — C# sizes it 1/3 of `oneThird` and pins it to oneThird's origin
            // (RelativeToView). note: absolute_layout cannot reference a sibling, so this is approximated
            // as a small proportional box (~0.11 = 0.33*0.33 of the parent) sitting at the silver box's
            // top-left, which lands in the same region the legacy view-relative constraint produced.
            black_.set_color(maui::graphics::color(0.05F, 0.05F, 0.05F));
            place(black_, 0.33, 0.33, 0.11, 0.11,
                  maui::layouts::absolute_layout_flags::position_proportional |
                      maui::layouts::absolute_layout_flags::size_proportional);

            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::absolute_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::box_view& centered()
        {
            return one_third_;
        }

    private:
        // Add `box` to the absolute layout with the given LayoutBounds (x, y, w, h) + flags. The flags
        // decide whether each component is read as a proportion (0..1) or an absolute device value.
        void place(maui::controls::box_view& box, double x, double y, double w, double h,
                   maui::layouts::absolute_layout_flags flags)
        {
            root_.add(box);
            root_.set_layout_bounds(box, maui::graphics::rect(x, y, w, h));
            root_.set_layout_flags(box, flags);
        }

        maui::controls::content_page page_;
        maui::controls::absolute_layout root_;
        maui::controls::box_view red_;
        maui::controls::box_view green_;
        maui::controls::box_view blue_;
        maui::controls::box_view yellow_;
        maui::controls::box_view one_third_;
        maui::controls::box_view black_;
    };
} // namespace maui::samples
