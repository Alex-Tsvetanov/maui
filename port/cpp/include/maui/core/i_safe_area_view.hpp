#pragma once
// maui::core::i_safe_area_view   <=  Microsoft.Maui.ISafeAreaView
// maui::core::i_safe_area_view2  <=  Microsoft.Maui.ISafeAreaView2 (the insets write-back half)
//
// The safe-area seam between a page and its native host (W2-24): the host consults ignore_safe_area()
// before honoring the device insets (C# MauiView.AdjustForSafeArea), and pushes the CURRENT native
// insets back through set_safe_area_insets (C# MauiView.SafeAreaInsetsDidChange →
// ISafeAreaView2.SafeAreaInsets). controls::content_page implements both over the iOSSpecific Page
// knobs (IgnoreSafeArea = !UsingSafeArea(); the insets store under "ios.Page.SafeAreaInsets" — exactly
// Page.cs's explicit interface implementations).

#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"

namespace maui::core
{
    class i_safe_area_view
    {
    public:
        virtual ~i_safe_area_view() = default;

        // C# ISafeAreaView.IgnoreSafeArea — true = lay out edge-to-edge (do NOT inset by the safe area).
        [[nodiscard]] virtual bool ignore_safe_area() const = 0;

    protected:
        i_safe_area_view() = default;
        i_safe_area_view(const i_safe_area_view&) = default;
        i_safe_area_view(i_safe_area_view&&) = default;
        i_safe_area_view& operator=(const i_safe_area_view&) = default;
        i_safe_area_view& operator=(i_safe_area_view&&) = default;
    };

    class i_safe_area_view2
    {
    public:
        virtual ~i_safe_area_view2() = default;

        // C# ISafeAreaView2.SafeAreaInsets (set) — the native host reports the realized insets here.
        virtual void set_safe_area_insets(const thickness& value) = 0;

        // C# ISafeAreaView2.GetSafeAreaRegionsForEdge(int edge) — the effective safe-area region for one
        // edge (0=Left, 1=Top, 2=Right, 3=Bottom). The iOS host (MauiView.AdjustForSafeArea) consults this
        // per edge to decide whether to inset by that edge's device safe area. The faithful default mirrors
        // ContentPage.cs's non-iOS branch (edge-to-edge — None) so a future implementer compiles unchanged.
        [[nodiscard]] virtual safe_area_regions get_safe_area_regions_for_edge(int edge) const
        {
            (void)edge;
            return safe_area_regions::none;
        }

        // C# `internal bool MauiView.AppliesSafeAreaAdjustments` (MauiView.cs:496) — "is this view currently
        // insetting by the safe area?". MAUI parks the bit on the NATIVE MauiView because that is where the
        // adjust lives, and reads it from a descendant's IsParentHandlingSafeArea() ancestor walk. The port
        // adjusts CROSS-PLATFORM (layout::measure/arrange), so the bit lives on the same object as the
        // adjust. ISafeAreaView2 is `internal` in C# too, so this seam is not public surface.
        //
        // Default false = "never insets, never blocks a descendant", which is exact for every current
        // implementer other than layout: content_page/border/content_view all default SafeAreaEdges to None
        // (see their default-value creators) and so never apply. An implementer that gains a non-None
        // default must override this, or a child layout could double-inset on the first pass.
        [[nodiscard]] virtual bool applies_safe_area_adjustments() const
        {
            return false;
        }

        // The insets this view is CURRENTLY applying (C# MauiView._safeArea / MauiScrollView._safeArea) —
        // the SETTER's counterpart, after the per-edge region filtering. Zero unless the view both received
        // insets and obeys them, so the default is exact for every implementer that never applies.
        //
        // WHO READS IT: a native host whose ViewGroup OWNS its children's layout, and which therefore has
        // to express the inset the way the platform does instead of relying on the cross-platform child
        // offset. On Android that is MAUI's own mechanism — SafeAreaExtensions.ApplyAdjustedSafeAreaInsetsPx
        // ends in `view.SetPadding(left, top, right, bottom)`, and MauiScrollView.OnLayout then just calls
        // base.OnLayout so the native ScrollView positions its child inside that padding. The port's
        // android scroll_view_handler::platform_arrange does the same, because android's ScrollView
        // (a FrameLayout) re-lays-out its single child at paddingTop and would otherwise DISCARD the
        // origin scroll_view::arrange gave it — measured: border_stroke rendered 136 px high, its first
        // label hidden behind the status bar.
        [[nodiscard]] virtual thickness applied_safe_area_insets() const
        {
            return {};
        }

    protected:
        i_safe_area_view2() = default;
        i_safe_area_view2(const i_safe_area_view2&) = default;
        i_safe_area_view2(i_safe_area_view2&&) = default;
        i_safe_area_view2& operator=(const i_safe_area_view2&) = default;
        i_safe_area_view2& operator=(i_safe_area_view2&&) = default;
    };
} // namespace maui::core
