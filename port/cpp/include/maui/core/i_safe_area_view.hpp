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

    protected:
        i_safe_area_view2() = default;
        i_safe_area_view2(const i_safe_area_view2&) = default;
        i_safe_area_view2(i_safe_area_view2&&) = default;
        i_safe_area_view2& operator=(const i_safe_area_view2&) = default;
        i_safe_area_view2& operator=(i_safe_area_view2&&) = default;
    };
} // namespace maui::core
