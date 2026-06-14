#pragma once
// maui::core::i_refresh_view  <=  Microsoft.Maui.IRefreshView
//
// The virtual-view contract for a container providing pull-to-refresh over scrollable content. Ported
// from src/Core/src/Core/IRefreshView.cs (IRefreshView : IView): IsRefreshing (settable — the platform
// writes it back when the user pulls; the control raises Refreshing + runs its refresh command from the
// setter), IsRefreshEnabled (whether the pull gesture is active), the RefreshColor paint for the spinner,
// and the scrollable Content. C# derives plain IView (NOT IContentView) and declares its own Content
// getter, so this contract does the same (Content is a direct getter, not the i_content_view base).

#include "maui/core/i_view.hpp"

namespace maui::graphics
{
    class paint;
}

namespace maui::core
{
    class i_refresh_view : public i_view
    {
    public:
        // C# IRefreshView.IsRefreshing — whether the view is loading; the setter is the platform's
        // write-back (the native pull-to-refresh control sets it true when the user pulls).
        [[nodiscard]] virtual bool is_refreshing() const = 0;
        virtual void set_is_refreshing(bool value) = 0;

        // C# IRefreshView.IsRefreshEnabled — whether the pull-to-refresh gesture is enabled.
        [[nodiscard]] virtual bool is_refresh_enabled() const = 0;

        // C# IRefreshView.RefreshColor — the loading-indicator color (null = platform default).
        [[nodiscard]] virtual const maui::graphics::paint* refresh_color() const = 0;

        // C# IRefreshView.Content — the scrollable content to refresh (null = none).
        [[nodiscard]] virtual i_view* content() const = 0;
    };
} // namespace maui::core
