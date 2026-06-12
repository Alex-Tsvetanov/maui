#pragma once
// maui::core::i_flyout_view  <=  Microsoft.Maui.IFlyoutView
//
// The two-pane contract a flyout page implements so its handler can host the flyout (menu) pane beside
// or over the detail pane. Ported from src/Core/src/Core/IFlyoutView.cs: Flyout, Detail, IsPresented
// (get + set — the native chrome can present/dismiss the flyout, so the seam is two-way), the computed
// FlyoutBehavior (Locked in split mode, Flyout otherwise), FlyoutWidth, and IsGestureEnabled.
//
// Like i_stack_navigation / i_tabbed_view this is a SIDE interface: it does not derive i_view (the
// flyout page already derives i_view through its view<> base) and the handler reaches it with a
// dynamic_cast from the virtual view. The members carry a flyout_ prefix so the contract names never
// collide with the control's own accessors (flyout() / detail() / is_presented()).

#include "maui/core/flyout_behavior.hpp"

namespace maui::core
{
    class i_view;

    class i_flyout_view
    {
    public:
        virtual ~i_flyout_view() = default;

        // IFlyoutView.Flyout — the flyout (menu/navigation) pane. Non-owning; null until set.
        [[nodiscard]] virtual i_view* flyout_view() const = 0;
        // IFlyoutView.Detail — the detail (content) pane. Non-owning; null until set.
        [[nodiscard]] virtual i_view* flyout_detail() const = 0;

        // IFlyoutView.IsPresented (get): whether the flyout pane is currently presented.
        [[nodiscard]] virtual bool flyout_is_presented() const = 0;
        // IFlyoutView.IsPresented (set): the native→virtual sync — the platform chrome presented or
        // dismissed the flyout (display-mode button, swipe, sidebar toggle) and the control records it.
        virtual void set_flyout_is_presented(bool value) = 0;

        // IFlyoutView.FlyoutBehavior — computed: Locked while the page should show split mode, Flyout
        // otherwise (FlyoutPage.cs's IFlyoutView.FlyoutBehavior).
        [[nodiscard]] virtual flyout_behavior flyout_behavior_value() const = 0;

        // IFlyoutView.FlyoutWidth — the requested flyout pane width; -1 = platform default.
        [[nodiscard]] virtual double flyout_width() const = 0;

        // IFlyoutView.IsGestureEnabled — whether swipe gestures may open the flyout.
        [[nodiscard]] virtual bool flyout_is_gesture_enabled() const = 0;

    protected:
        i_flyout_view() = default;
        i_flyout_view(const i_flyout_view&) = default;
        i_flyout_view(i_flyout_view&&) = default;
        i_flyout_view& operator=(const i_flyout_view&) = default;
        i_flyout_view& operator=(i_flyout_view&&) = default;
    };
} // namespace maui::core
