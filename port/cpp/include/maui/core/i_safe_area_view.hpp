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

    protected:
        i_safe_area_view2() = default;
        i_safe_area_view2(const i_safe_area_view2&) = default;
        i_safe_area_view2(i_safe_area_view2&&) = default;
        i_safe_area_view2& operator=(const i_safe_area_view2&) = default;
        i_safe_area_view2& operator=(i_safe_area_view2&&) = default;
    };
} // namespace maui::core
