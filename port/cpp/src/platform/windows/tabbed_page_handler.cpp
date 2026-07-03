// tabbed_page_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// hosting the CURRENT tab page's content plus a TOP tab bar (a horizontal StackPanel of one Button per
// tab). The windows twin of src/platform/android/tabbed_page_handler.cpp (current-tab content + a plain
// tab bar of TextViews) and the real-native sibling of the headless tab mirror
// (src/platform/headless/tabbed_page_handler.cpp).
//
// C# ORIGIN: the cross-platform TabbedViewHandler is a stub (CreatePlatformView throws); the Windows
// TabbedPage chrome is Controls-layer plumbing (Controls/src/Core/TabbedPage/TabbedPage.Windows.cs — a
// root NavigationView in Top pane mode whose MenuItems are the tab pages, driven through
// MauiNavigationView). That NavigationView infra has not reached this backend, so the host renders the
// same library-independent shape the android twin ports: the current tab's content + a plain tab bar.
//
// THE HOST: a plain Canvas (the port's manual-frame container — content_page/layout twin). set_pages
// rebuilds the tab bar (a horizontal StackPanel of Buttons titled with the tab titles) and re-hosts the
// CURRENT page's native element (the page fills the host through the tabbed_page control's own
// host-relative arrange); set_current swaps the visible content child + re-highlights the selected tab;
// update_bar applies the bar/text/selected colors. platform_arrange frames the host, then explicitly
// frames the bar as a TOP strip (Canvas children are manually framed; the StackPanel internally lays out
// its Buttons).
//
// LIVE TAB SWITCH (the android twin's deferred half, implemented here): each tab Button's Click routes
// through the platform's on_tab_select callback into i_tabbed_view::on_tab_selected — the native→virtual
// selection seam the apple/ios delegates wire (C#'s NavigationView.SelectionChanged → CurrentPage sync).
// The control then makes that page current and the "current_page" map re-hosts it.
//
// DOCUMENTED DEVIATIONS from the C# oracle (infrastructure gaps of this first cut, not behavior guesses):
//   - The bar is a plain StackPanel of stock Buttons, NOT the MauiNavigationView top-tabs chrome (header
//     note). It sits as a TOP strip (WinUI's TabbedPage tabs ride the top pane) with a fixed 48pt height
//     (the NavigationView top-pane default); tabs are NOT equal-width (StackPanel auto-sizes each title).
//   - ALL tab pages are arranged over the full bounds by tabbed_page::arrange (the controller-insets-the-
//     content convention), but only the CURRENT page is hosted as a child here; a tab switch re-hosts.
//     The content fills the host and the bar overlays its top strip (the android bottom-strip overlay,
//     mirrored to the windows top-tabs shape).
//   - The four bar colors are pushed where the stock widgets express them (bar background → the
//     StackPanel's Panel.Background; text/selected/unselected → the Buttons' Foreground); colors the
//     developer never set fall back to modest stock defaults (light-gray strip, blue selected, dark
//     unselected — the android twin's constants). The BarBackground BRUSH is mirrored (a non-owning
//     aliasing borrow) but only a SOLID fill is painted (gradient bar fills are the deferred half).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also runs
// the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches the
// construction failure and keeps native null, while the headless tab mirror (hosted_pages / tab_titles /
// hosted_current / selected_index / the four colors / the brush borrow) is ALWAYS maintained — and the
// on_tab_select callback stays invokable — so that suite observes exactly the headless partial's tracking.

#include "maui/core/tabbed_page_handler.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ButtonBase.Click consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>
#include <winrt/base.h>

#include "maui/controls/brushes/brush.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wnative = maui::platform::win;

    // The tab-bar strip height in pt — the WinUI NavigationView TOP pane height default (the windows
    // TabbedPage's tabs ride the top pane).
    constexpr double k_tab_bar_height = 48.0;
    // Default tab foregrounds when the developer set none: a near-black unselected, a system-blue
    // selected (the android twin's constants, shared for cross-backend parity of the plain bar).
    constexpr winrt::Windows::UI::Color k_default_selected{.A = 0xFF, .R = 0x00, .G = 0x7A, .B = 0xFF};
    constexpr winrt::Windows::UI::Color k_default_unselected{.A = 0xFF, .R = 0x33, .G = 0x33, .B = 0x33};
    // The default bar background when the developer set none: a light gray strip.
    constexpr winrt::Windows::UI::Color k_default_bar_background{.A = 0xFF, .R = 0xF8, .G = 0xF8, .B = 0xF8};

    // The page's native UIElement, via its view-handler's native_view() (C#'s ToPlatform()).
    [[nodiscard]] mux::UIElement native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        return wnative::borrow_as<mux::UIElement>(handler->native_view());
    }

} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the host + the bar + each tab Button (the wnative shape of the
    // pimpl-owned-native doctrine; the android twin deletes its global refs here). The buttons' Click
    // tokens are revoked first so no handler outlives its platform peer. The hosted tab pages are owned
    // by their own page handlers (non-owning children) — nothing to release for those.
    tabbed_page_platform::~tabbed_page_platform()
    {
        for (std::size_t i = 0; i < tab_buttons.size(); ++i)
        {
            if (auto button = wnative::borrow<muxc::Button>(tab_buttons[i]))
            {
                if (i < tab_click_tokens.size() && tab_click_tokens[i] != 0)
                {
                    button.Click(winrt::event_token{tab_click_tokens[i]});
                }
            }
            wnative::release(tab_buttons[i]);
        }
        tab_buttons.clear();
        tab_click_tokens.clear();
        wnative::release(tab_bar);
        wnative::release(native);
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Base body FIRST (the XAML-less suite observes the headless mirror), then the
    // real host (the content_page dual-path pattern). ----

    void tabbed_page_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void tabbed_page_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void tabbed_page_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void tabbed_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto host = wnative::borrow<muxc::Canvas>(native);
        if (host == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Panel branch: panel.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            host.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        host.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints — the base mirror above keeps the borrow observable.
    }

    std::unique_ptr<tabbed_page_platform> tabbed_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<tabbed_page_platform>();
        try
        {
            // The port's manual-frame Canvas tab host (C#'s Windows TabbedPage chrome is a Controls-layer
            // root NavigationView — deferred; header deviations).
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~tabbed_page_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void tabbed_page_handler::on_connect_handler(tabbed_page_platform& platform)
    {
        // The native→virtual selection seam (C#'s NavigationView.SelectionChanged → CurrentPage sync,
        // apple/ios delegate twin): each tab Button's Click routes here with its index; the control makes
        // that page current. Wired even XAML-less so the cross-platform suite can drive the seam (the
        // button partial's callback shape).
        platform.on_tab_select = [this](std::size_t index) {
            auto* view = virtual_view();
            if (view == nullptr)
            {
                return;
            }
            if (auto* tabbed = dynamic_cast<i_tabbed_view*>(view))
            {
                tabbed->on_tab_selected(index);
            }
        };
    }

    void tabbed_page_handler::set_pages(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapItemsSource reads the pages + titles).
        platform->hosted_pages = tabbed->tabbed_pages();
        platform->tab_titles = tabbed->tabbed_titles();

        if (platform->native == nullptr)
        {
            return;
        }
        auto host = wnative::borrow<muxc::Canvas>(platform->native);
        if (host == nullptr)
        {
            return;
        }

        // Clear the old host children (the previous content + bar), revoke the old Click handlers, and
        // release the old buttons + bar (the android twin's rebuild sweep).
        host.Children().Clear();
        for (std::size_t i = 0; i < platform->tab_buttons.size(); ++i)
        {
            if (auto button = wnative::borrow<muxc::Button>(platform->tab_buttons[i]))
            {
                if (i < platform->tab_click_tokens.size() && platform->tab_click_tokens[i] != 0)
                {
                    button.Click(winrt::event_token{platform->tab_click_tokens[i]});
                }
            }
            wnative::release(platform->tab_buttons[i]);
        }
        platform->tab_buttons.clear();
        platform->tab_click_tokens.clear();
        wnative::release(platform->tab_bar);

        // (1) Build the bar (a horizontal StackPanel) + one Button per tab, wiring each Click to the
        //     on_tab_select seam; the selection + colors are realized by set_current / update_bar below.
        try
        {
            if (!platform->hosted_pages.empty())
            {
                const muxc::StackPanel bar;
                bar.Orientation(muxc::Orientation::Horizontal);
                // A bar background so the strip reads as a tab bar (overridden by update_bar when the
                // developer set BarBackgroundColor).
                bar.Background(muxm::SolidColorBrush{k_default_bar_background});
                auto* peer = platform;
                for (std::size_t i = 0; i < platform->tab_titles.size(); ++i)
                {
                    const muxc::Button button;
                    button.Content(winrt::box_value(wnative::to_hstring_utf8(platform->tab_titles[i])));
                    button.Height(k_tab_bar_height);
                    // The live tab switch: Click → on_tab_select(index) → i_tabbed_view::on_tab_selected
                    // (the peer is the platform struct, whose heap address is stable — the button partial's
                    // event-routing shape; the token is revoked before the button is released).
                    const std::size_t index = i;
                    const winrt::event_token click_token = button.Click(
                        [peer, index](const winrt::Windows::Foundation::IInspectable&, const mux::RoutedEventArgs&) {
                            if (peer->on_tab_select)
                            {
                                peer->on_tab_select(index);
                            }
                        });
                    bar.Children().Append(button);
                    platform->tab_buttons.push_back(wnative::store(button));
                    platform->tab_click_tokens.push_back(click_token.value);
                }
                platform->tab_bar = wnative::store(bar); // released in ~tabbed_page_platform / next rebuild
            }
        }
        catch (const winrt::hresult_error&)
        {
            // Bar construction failed — the content still hosts below; the mirrors stay authoritative.
        }

        // (2) Host the current page's content + add the bar on top, then apply selection + bar colors.
        set_current(view);
        update_bar(view);
    }

    void tabbed_page_handler::set_current(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapCurrentPage).
        platform->hosted_current = tabbed->tabbed_current_page();
        platform->selected_index = -1;
        for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
        {
            if (platform->hosted_pages[i] == platform->hosted_current)
            {
                platform->selected_index = static_cast<int>(i);
                break;
            }
        }

        if (platform->native == nullptr)
        {
            return;
        }
        auto host = wnative::borrow<muxc::Canvas>(platform->native);
        if (host == nullptr)
        {
            return;
        }
        // Re-host: clear, append the current page's content, then the bar on top (Canvas z-order = the
        // append order; the bar + buttons stay pinned by the platform's strong refs across the Clear —
        // the android remove-content-re-add-bar swap, collapsed to clear-and-rebuild).
        host.Children().Clear();
        if (platform->hosted_current != nullptr)
        {
            if (auto element = native_child(*platform->hosted_current))
            {
                wnative::detach_from_parent(element);
                host.Children().Append(element);
            }
        }
        if (auto bar = wnative::borrow_as<mux::UIElement>(platform->tab_bar))
        {
            wnative::detach_from_parent(bar);
            host.Children().Append(bar);
        }

        // Re-highlight the selected tab (the colors are re-applied here so a tab switch recolors).
        update_bar(view);
    }

    void tabbed_page_handler::update_bar(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# MapBar* keys). The Brush bar fill is captured as a
        // NON-OWNING borrow (aliasing shared_ptr, empty owner — the control owns the brush).
        platform->bar_background_color = tabbed->tab_bar_background_color();
        platform->bar_text_color = tabbed->tab_bar_text_color();
        platform->selected_tab_color = tabbed->tab_selected_color();
        platform->unselected_tab_color = tabbed->tab_unselected_color();
        const std::optional<maui::controls::brush*> brush = tabbed->tab_bar_background_brush();
        platform->bar_background_brush =
            (brush.has_value() && *brush != nullptr)
                ? std::optional{std::shared_ptr<maui::controls::brush>{std::shared_ptr<void>{}, *brush}}
                : std::nullopt;

        if (platform->native == nullptr || platform->tab_bar == nullptr)
        {
            return;
        }
        auto bar = wnative::borrow<muxc::StackPanel>(platform->tab_bar);
        if (bar == nullptr)
        {
            return;
        }

        // The resolved colors: developer-set wins, else the stock defaults. selected falls back to
        // bar_text_color, then the system-blue default; unselected falls back to bar_text_color, then the
        // dark default (the android twin's collapse of C#'s SetTabIconColors convention).
        const winrt::Windows::UI::Color selected_color =
            platform->selected_tab_color.has_value() ? wnative::to_ui_color(*platform->selected_tab_color)
            : platform->bar_text_color.has_value()   ? wnative::to_ui_color(*platform->bar_text_color)
                                                     : k_default_selected;
        const winrt::Windows::UI::Color unselected_color =
            platform->unselected_tab_color.has_value() ? wnative::to_ui_color(*platform->unselected_tab_color)
            : platform->bar_text_color.has_value()     ? wnative::to_ui_color(*platform->bar_text_color)
                                                       : k_default_unselected;

        // The bar background: BarBackgroundColor wins, else the SOLID BarBackground brush, else the
        // default strip color. Only a solid fill is painted (gradient bar fills are the deferred half —
        // header); the brush's resolved background color stands in for a SolidColorBrush.
        winrt::Windows::UI::Color bar_background = k_default_bar_background;
        if (platform->bar_background_color.has_value())
        {
            bar_background = wnative::to_ui_color(*platform->bar_background_color);
        }
        else if (platform->bar_background_brush.has_value() && *platform->bar_background_brush != nullptr)
        {
            if (const auto* paint = dynamic_cast<const maui::graphics::paint*>(platform->bar_background_brush->get()))
            {
                bar_background = wnative::to_ui_color(paint->background_color());
            }
        }
        bar.Background(muxm::SolidColorBrush{bar_background});

        // Recolor each tab Button by its selected state (selected_index < 0 highlights the first tab,
        // the android twin's fresh-host convention).
        for (std::size_t i = 0; i < platform->tab_buttons.size(); ++i)
        {
            const bool selected = static_cast<int>(i) == platform->selected_index ||
                                  (platform->selected_index < 0 && i == 0);
            if (auto button = wnative::borrow<muxc::Button>(platform->tab_buttons[i]))
            {
                button.Foreground(muxm::SolidColorBrush{selected ? selected_color : unselected_color});
            }
        }
    }

    maui::graphics::size tabbed_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The tabbed page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void tabbed_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the host to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);

        // Frame the tab bar as a TOP strip: the host is a Canvas (manual frames), so the bar — a real
        // StackPanel that internally lays out its Buttons — is pinned at the top of the host in local
        // 0-origin coordinates (the android twin's bottom-strip layout, mirrored to the windows top-tabs
        // shape; the content, arranged over the full bounds by the control, sits underneath).
        if (auto bar = wnative::borrow_as<mux::FrameworkElement>(platform->tab_bar))
        {
            muxc::Canvas::SetLeft(bar, 0.0);
            muxc::Canvas::SetTop(bar, 0.0);
            bar.Width(frame.width);
            bar.Height(k_tab_bar_height);
        }
    }
} // namespace maui::core
