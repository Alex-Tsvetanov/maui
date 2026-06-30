#pragma once
// maui::core::tabbed_page_handler  <=  Microsoft.Maui.Handlers.TabbedViewHandler
//
// The handler behind a tabbed_page: it owns the native tab host and keeps its tab items (one per child
// page, titled with the page's Title), the selected tab, and the bar styling in sync with the virtual
// view. C#'s cross-platform TabbedViewHandler is a stub (CreatePlatformView throws; each platform's
// chrome comes from platform factories / the compat renderer) driven through control-typed mapper
// replacements (TabbedPage.Mapper.cs) — the port keeps the same KEYS ("items_source" / "current_page" /
// the four bar colors) as ordinary mapper entries reading the maui::core::i_tabbed_view seam (see that
// header for why the contract is widened).
//
// Per-backend platform recipe (the partial split):
//   headless — mirrors only (hosted_pages / tab_titles / hosted_current / selected_index / colors).
//   ios      — a real UITabBarController: each page's native UIView is wrapped in a child
//              UIViewController (the controller hierarchy the task asserts), tab titles via
//              tabBarItem.title, selectedIndex synced both ways (programmatic set + the
//              UITabBarControllerDelegate's didSelectViewController → i_tabbed_view::on_tab_selected).
//   apple    — a real NSTabViewController: one NSTabViewItem per page (each item's view controller's
//              view hosts the page's NSView), selectedTabViewItemIndex synced both ways through an
//              NSTabViewController subclass forwarding didSelectTabViewItem. AppKit's tab chrome has no
//              bar-color API — the four colors stay mirror-only there (documented deviation).
//
// set_pages rebuilds the whole tab set (C#'s MapItemsSource semantics — the control refreshes
// "items_source" on every pages change AND on a child Title change, per TabbedPage's
// OnHandlerChangingCore wiring), then re-applies the selection and bar styling.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class brush; // the bar-background fill (owned by the control; the platform mirrors a non-owning borrow)
} // namespace maui::controls

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; the real backends override update_* on the tab host's view).
    struct tabbed_page_platform : view_platform_base
    {
        tabbed_page_platform() = default;
        ~tabbed_page_platform() override; // backend-defined: releases the retained native views
        tabbed_page_platform(const tabbed_page_platform&) = delete;
        tabbed_page_platform(tabbed_page_platform&&) = delete;
        tabbed_page_platform& operator=(const tabbed_page_platform&) = delete;
        tabbed_page_platform& operator=(tabbed_page_platform&&) = delete;

        void* native = nullptr; // the tab host's root view (the controller's .view on the real backends)

        // The seam mirrors (headless-observable; the real backends ALSO drive the controller):
        std::vector<i_view*> hosted_pages;   // the tab pages, in tab order
        std::vector<std::string> tab_titles; // the tab item titles (each page's Title)
        i_view* hosted_current = nullptr;    // the selected page
        int selected_index = -1;             // the selected tab index (-1 = none)
        std::optional<maui::graphics::color> bar_background_color;
        std::optional<maui::graphics::color> bar_text_color;
        std::optional<maui::graphics::color> selected_tab_color;
        std::optional<maui::graphics::color> unselected_tab_color;
        // C# TabbedPage.BarBackground (the Brush bar fill): a NON-OWNING borrow of the control's brush
        // (the seam yields a borrowed pointer; the mirror is an aliasing shared_ptr with an empty owner so
        // it observes pointer identity without retaining). nullopt when the developer never set it.
        std::optional<std::shared_ptr<maui::controls::brush>> bar_background_brush;
        // The InvalidateGradientBrushRequested subscription on the current GRADIENT bar background (so a
        // stop change repaints). Held while a gradient brush is the bar fill; cleared when it changes to a
        // non-gradient / null brush. Default-constructed = no subscription. Real backends (ios) re-subscribe
        // in update_bar; headless / apple leave it empty (they capture but do not paint).
        maui::core::scoped_connection bar_background_invalidate_token;

#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        // The retained native slots shared by the two real-native twins.
        void* controller = nullptr; // NSTabViewController / UITabBarController (retained)
        void* delegate = nullptr;   // the selection trampoline (UITabBarControllerDelegate proxy on ios;
                                    // unused on apple — the controller subclass forwards selection itself)
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the tab controller's NSView (defined in
        // src/platform/apple/tabbed_page_handler.mm). is_enabled keeps the base mirror (plain NSView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the tab controller's UIView (defined in
        // src/platform/ios/tabbed_page_handler.mm). is_enabled keeps the base mirror (plain UIView).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (src/platform/android/tabbed_page_handler.cpp): the host is a real
        // dev.mauicpp.MauiLayout hosting the CURRENT tab's content + a bottom tab bar (a horizontal
        // android.widget.LinearLayout of one android.widget.TextView per tab). The Material
        // BottomNavigationView / TabLayout+ViewPager2 is unavailable on this AAR-less backend, so the bar
        // is a plain LinearLayout of TextViews; the live tap-to-switch listener is the documented deviation
        // (see the .cpp header). The retained native slots:
        void* tab_bar = nullptr;      // the bar LinearLayout (global ref; rebuilt by set_pages)
        std::vector<void*> tab_views; // one TextView per tab (global refs; rebuilt by set_pages)

        // Each override calls the view_platform_base body FIRST (the VM-less cross-platform suite observes
        // the headless mirror), then pushes to the real host when one exists. Visibility/opacity/
        // automation_id push directly; transform/flow_direction/background/semantics through the shared
        // android ops. is_enabled keeps only the base mirror (a plain ViewGroup host has no enabled state,
        // matching the apple/ios twins).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class tabbed_page_handler : public view_handler<tabbed_page_handler, i_view, tabbed_page_platform>
    {
    public:
        tabbed_page_handler();

        static property_mapper<i_view, tabbed_page_handler>& mapper();
        static command_mapper<i_view, tabbed_page_handler>& command_mapper();

        static std::unique_ptr<tabbed_page_platform> create_platform_view();

        // The native UITabBarController this handler owns (the platform's `controller` slot), so the window
        // host can set the UIWindow's rootViewController to it — activating the tab controller's child-VC
        // lifecycle (without which the tabs / tab bar never lay out / render). Mirrors C#'s
        // IPlatformViewHandler.ViewController. iOS-only (the `controller` slot exists on the real-native
        // builds); returns null on headless. Defined per backend.
        [[nodiscard]] void* root_view_controller() const override;

        // Wire the native selection trampoline back to this handler (real backends; headless empty).
        void on_connect_handler(tabbed_page_platform& platform);

        // The tabbed page computes its size from its current page, not the handler (the container
        // convention — layout_handler / content_page_handler / navigation_page_handler).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Rebuild the native tab items from the view's pages + titles, then re-apply selection and bar
        // styling (defined per backend).
        void set_pages(i_view& view);
        // Select the tab matching the view's current page (defined per backend).
        void set_current(i_view& view);
        // Push the four bar colors to the native chrome (defined per backend).
        void update_bar(i_view& view);

        // "items_source" — C# MapItemsSource (+ the PagesChanged / child-Title refresh wiring).
        static void map_items_source(tabbed_page_handler& handler, i_view& view);
        // "current_page" — C# MapCurrentPage.
        static void map_current_page(tabbed_page_handler& handler, i_view& view);
        // The four bar-styling keys all re-apply the whole bar (C# MapBarBackgroundColor etc.).
        static void map_bar(tabbed_page_handler& handler, i_view& view);
    };
} // namespace maui::core
