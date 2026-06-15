#pragma once
// maui::core::i_tabbed_view  <=  Microsoft.Maui.ITabbedView
//
// The tab-host contract a tabbed page implements so its handler can build the native tab chrome. C#'s
// ITabbedView is an empty marker (the Controls TabbedPage drives the TabbedViewHandler through control-
// typed mapper replacements — TabbedPage.Mapper.cs); the reflection-free port cannot down-cast the
// handler's virtual view to the controls layer, so the state those control-typed mappers read (the
// children + their titles, the current page, the bar colors) is surfaced HERE, the same collapse the
// port applied to i_stack_navigation (the IToolbar info folded onto the seam the handler already
// dynamic_casts to).
//
// Like i_stack_navigation, this is a SIDE interface: it does not derive i_view (the tabbed page already
// derives i_view through its view<> base), and the handler reaches it with a dynamic_cast from the
// virtual view. on_tab_selected is the native→virtual back-channel: the platform tab chrome reports a
// user tab selection and the control makes that page current (C#'s renderer ViewControllerSelected →
// CurrentPage sync).

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class brush; // the developer-facing fill the bar background is set to (owned by the control)
} // namespace maui::controls

namespace maui::core
{
    class i_view;

    class i_tabbed_view
    {
    public:
        virtual ~i_tabbed_view() = default;

        // The tab pages, in tab order (MultiPage<T>.Children as IViews). Non-owning.
        [[nodiscard]] virtual std::vector<i_view*> tabbed_pages() const = 0;

        // The tab titles, parallel to tabbed_pages() (each page's Page.Title — the native tab item text).
        [[nodiscard]] virtual std::vector<std::string> tabbed_titles() const = 0;

        // The currently selected page (MultiPage<T>.CurrentPage), or null when there are no tabs.
        [[nodiscard]] virtual i_view* tabbed_current_page() const = 0;

        // Native→virtual selection sync: the platform tab chrome selected the tab at `index` (a user
        // tap); the control makes that page current. Out-of-range indices are ignored.
        virtual void on_tab_selected(std::size_t index) = 0;

        // ---- the tab-bar styling (C# TabbedPage Bar*/SelectedTabColor/UnselectedTabColor) ----
        // Each is nullopt when the developer never set it, so the native bar keeps its system default
        // (C# default(Color) = null) — the navigation_bar_* convention.
        [[nodiscard]] virtual std::optional<maui::graphics::color> tab_bar_background_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> tab_bar_text_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> tab_selected_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> tab_unselected_color() const = 0;

        // ---- the tab-bar background BRUSH (C# TabbedPage.BarBackground — IBarElement.BarBackground) ----
        // The Brush the developer set as the bar fill (SolidColorBrush / LinearGradientBrush /
        // RadialGradientBrush), as a BORROWED pointer — the control owns the brush; the handler must NOT
        // retain it. nullopt when the developer never set it (C# default(Brush) = null), so the native bar
        // keeps its color-driven default. Distinct from tab_bar_background_color above: BarBackground (a
        // Brush) overlays a CALayer; the color path drives the appearance background.
        [[nodiscard]] virtual std::optional<maui::controls::brush*> tab_bar_background_brush() const = 0;

    protected:
        i_tabbed_view() = default;
        i_tabbed_view(const i_tabbed_view&) = default;
        i_tabbed_view(i_tabbed_view&&) = default;
        i_tabbed_view& operator=(const i_tabbed_view&) = default;
        i_tabbed_view& operator=(i_tabbed_view&&) = default;
    };
} // namespace maui::core
