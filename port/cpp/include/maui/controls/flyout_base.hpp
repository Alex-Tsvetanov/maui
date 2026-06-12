#pragma once
// maui::controls::flyout_base  <=  Microsoft.Maui.Controls.FlyoutBase
//
// The abstract base of attachable flyouts + the ContextFlyout attached property's static accessors.
// Ported from src/Controls/src/Core/Menu/FlyoutBase.cs: SetContextFlyout/GetContextFlyout attach a
// flyout to a view as its right-click/long-press context menu. The attached storage lives ON the view
// (view<>::set_context_flyout — the analog of the C# attached BindableProperty's per-instance store),
// which also runs C#'s propertyChanged work: AddRemoveLogicalChildren (so the flyout inherits the
// view's BindingContext) and the handler update that materializes the native menu.

#include "maui/controls/element.hpp"
#include "maui/core/i_flyout.hpp"

namespace maui::controls
{
    // i_flyout is a VIRTUAL base: menu_flyout reaches it both through this class and through its
    // i_menu_flyout core contract, and the bases must collapse to one subobject for the
    // i_flyout*-typed attach surface to stay unambiguous.
    class flyout_base : public element, public virtual maui::core::i_flyout
    {
    public:
        // C# FlyoutBase.SetContextFlyout(BindableObject, FlyoutBase). TView is any view<...> control
        // (the template stands in for C#'s any-BindableObject signature; only views materialize it).
        template <class TView> static void set_context_flyout(TView& view, flyout_base* flyout)
        {
            view.set_context_flyout(flyout);
        }

        // C# FlyoutBase.GetContextFlyout(BindableObject) — the attached flyout, or null.
        template <class TView> [[nodiscard]] static maui::core::i_flyout* get_context_flyout(const TView& view)
        {
            return view.context_flyout();
        }

    protected:
        flyout_base() = default;
    };
} // namespace maui::controls
