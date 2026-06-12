#pragma once
// maui::core::window_handler  <=  Microsoft.Maui.Handlers.WindowHandler
//
// The native host behind a window (the i_window virtual view). UNLIKE the control handlers, a window is
// NOT an i_view (it has no measure/arrange, no generic-IView visual properties), so this handler derives
// the LIGHTWEIGHT i_element_handler directly — not the CRTP view_handler (whose Virtual must be an
// i_view). It services the Core i_window contract (never the concrete controls::window), exactly as MAUI's
// WindowHandler services IWindow. Ported from WindowHandler.cs + WindowHandler.iOS.cs (mainline MAUI has
// no AppKit WindowHandler — macOS there is Mac Catalyst/UIKit; the AppKit recipe is translated from the
// UIWindow one).
//
// The window owns this handler (PROFILE §8); the handler holds a NON-owning back-reference to the window
// (an i_window*) and OWNS the native window via a window_platform pimpl. On connect it creates the native
// window, hosts the root page's native view as the content view, and runs the property mapper (Title /
// Content / X / Y / Width / Height — WindowHandler.Mapper's MapTitle/MapContent/MapX/…). The native
// window's notifications map BACK to the window lifecycle: did-become-main → send_activated, will-close →
// send_destroying (a thin NSWindowDelegate/notification trampoline on Apple).
//
// Same partial-class split + single cross-platform window_platform struct as the view handlers: the mapper
// TABLE + ctor + the cross-platform connect/disconnect/update_value/invoke lifecycle live here
// (window_handler.cpp); the platform recipe (create the native window, host the page, push title/geometry,
// wire the notifications) lives per backend under src/platform/<backend>/window_handler.{cpp,mm}.

#include <any>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/property_mapper.hpp"

namespace maui::core
{
    class i_maui_context;
    // --- chrome (W1-11) forwards: the window chrome contracts the mapper additions below consume ---
    class i_toolbar;
    class i_menu_bar;
    class i_title_bar;
    // --- end chrome (W1-11) ---

    // The pimpl that owns the native window (PROFILE §8). A single cross-platform struct (like the view
    // handlers' *_platform): the native slot + a headless mirror of what the native host tracks (the hosted
    // window + the title + content-hosted + activated flags), so the headless tests can observe the host
    // without a real window. Backend-defined destructor releases the retained native window on Apple.
    struct window_platform
    {
        window_platform() = default;
        ~window_platform(); // backend-defined: releases the retained NSWindow on Apple
        window_platform(const window_platform&) = delete;
        window_platform(window_platform&&) = delete;
        window_platform& operator=(const window_platform&) = delete;
        window_platform& operator=(window_platform&&) = delete;

        void* native = nullptr; // the native NSWindow (Apple) — null on headless

        // ---- headless mirror (the host's observable state; the Apple build ALSO drives the real NSWindow) ----
        i_window* hosted_window = nullptr; // the virtual view (set on connect)
        std::string title;                 // the last title pushed (MapTitle)
        bool content_hosted = false;       // whether the root page's native view is hosted (MapContent)
        bool activated = false;            // mirrors NSWindow main/key on the headless backend

        // --- chrome (W1-11): the window chrome mirrors + the retained native chrome slots -------------
        // NON-owning borrows of the window's chrome (null = none); every backend records these, and the
        // Apple build additionally materializes the real NSToolbar / NSMenu main menu / titlebar
        // accessory from them. On iOS all three stay mirror-only by design: the toolbar items surface
        // through the navigation bar (navigation_page_handler), and C# materializes menu bars / title
        // bars on desktop (Windows/Catalyst) only — documented no-ops.
        i_toolbar* hosted_toolbar = nullptr;     // MapToolbar (IToolbarElement.Toolbar)
        i_menu_bar* hosted_menu_bar = nullptr;   // MapMenuBar (IMenuBarElement.MenuBar)
        i_title_bar* hosted_title_bar = nullptr; // MapTitleBar (IWindow.TitleBar)
#ifdef MAUI_PLATFORM_APPLE
        void* chrome_toolbar = nullptr;          // retained NSToolbar (window.toolbar)
        void* chrome_toolbar_delegate = nullptr; // retained NSToolbarDelegate trampoline (items + clicks)
        void* chrome_main_menu = nullptr;        // retained NSMenu (assigned to NSApp.mainMenu when present)
        void* chrome_menu_target = nullptr;      // retained menu-item target trampoline (click → send_clicked)
        void* chrome_title_bar = nullptr;        // retained NSTitlebarAccessoryViewController
#endif
        // --- end chrome (W1-11) ------------------------------------------------------------------------

#ifdef MAUI_PLATFORM_APPLE
        // The trampoline (an NSWindowDelegate-style observer) that maps the native window notifications back
        // to the window lifecycle. Held as a void* (retained) so the cross-platform struct stays Obj-C-free;
        // released in the destructor. Defined in window_handler.mm.
        void* notification_trampoline = nullptr;
#endif

#ifdef MAUI_PLATFORM_IOS
        // The iOS twin of the Apple slot: the lifecycle proxy observing the UIApplication notifications
        // (did-become-active → send_activated, will-enter-foreground → send_resumed, did-enter-background →
        // send_stopped, will-terminate → send_destroying — the AppHostBuilderExtensions.iOS non-scene
        // lifecycle map) AND KVO-observing the UIWindow's frame (WindowHandler.iOS's FrameObserverProxy →
        // frame_changed). Held as a void* (retained) so the cross-platform struct stays Obj-C-free; released
        // (after un-observing) in the destructor. Defined in src/platform/ios/window_handler.mm.
        void* notification_trampoline = nullptr;
#endif
    };

    class window_handler : public i_element_handler
    {
    public:
        window_handler();
        ~window_handler() override;
        window_handler(const window_handler&) = delete;
        window_handler(window_handler&&) = delete;
        window_handler& operator=(const window_handler&) = delete;
        window_handler& operator=(window_handler&&) = delete;

        // C# WindowHandler.Mapper — Title / Content / X / Y / Width / Height. NON-chained: a window has no
        // generic-IView property set (it is not an i_view), so it does NOT chain the view_mapper (C#'s
        // ElementHandler.ElementMapper is empty in this port).
        static property_mapper<i_window, window_handler>& mapper();

        static std::unique_ptr<window_platform> create_platform_view();

        // ---- i_element_handler (hand-written: a window is not a view, so no CRTP view_handler base) ----
        void set_maui_context(i_maui_context* context) override;
        void set_virtual_view(i_element& view) override;
        void update_value(std::string_view property) override;
        void invoke(std::string_view command, const std::any& args = {}) override;
        void disconnect_handler() override;

        [[nodiscard]] void* platform_view() const override
        {
            return platform_view_.get();
        }
        [[nodiscard]] i_element* virtual_view() const override
        {
            return virtual_view_;
        }
        [[nodiscard]] i_maui_context* maui_context() const override
        {
            return maui_context_;
        }

        // The typed platform-view accessor (the window_platform pimpl) for the mapper functions + tests.
        [[nodiscard]] window_platform* typed_platform_view() const
        {
            return platform_view_.get();
        }
        // The typed virtual view (the window contract), or null if disconnected.
        [[nodiscard]] i_window* window_view() const
        {
            return window_view_;
        }

        // ---- mapper functions (WindowHandler.MapTitle / MapContent / MapX / MapY / MapWidth / MapHeight) ----
        static void map_title(window_handler& handler, i_window& view);
        static void map_content(window_handler& handler, i_window& view);
        static void map_x(window_handler& handler, i_window& view);
        static void map_y(window_handler& handler, i_window& view);
        static void map_width(window_handler& handler, i_window& view);
        static void map_height(window_handler& handler, i_window& view);

        // --- chrome (W1-11): WindowHandler.MapToolbar / MapMenuBar / MapTitleBar ------------------------
        // Each cross-casts the i_window to its chrome element interface (i_toolbar_element /
        // i_menu_bar_element / i_title_bar_element — C#'s `window as IToolbarElement` probes) and hands
        // the chrome to the per-backend apply_* recipe (real NSToolbar / NSMenu main menu / titlebar
        // accessory on AppKit; mirror-only on headless and iOS — see window_platform above). The apply_*
        // are defined per backend in src/platform/<backend>/window_handler.{cpp,mm}.
        static void map_toolbar(window_handler& handler, i_window& view);
        static void map_menu_bar(window_handler& handler, i_window& view);
        static void map_title_bar(window_handler& handler, i_window& view);
        // const like disconnect(): each touches only the pimpl (and the native window it owns).
        void apply_toolbar(i_toolbar* toolbar) const;
        void apply_menu_bar(i_menu_bar* menu_bar) const;
        void apply_title_bar(i_title_bar* title_bar) const;
        // --- end chrome (W1-11) ------------------------------------------------------------------------

    private:
        // The backend recipe: create the native window, host the root page's native view as the content
        // view, push the title, move/size to a frame, and wire the native window notifications back to the
        // window lifecycle (defined per backend in window_handler.{cpp,mm}).
        void connect();          // host page + wire notifications (C# ConnectHandler)
        void disconnect() const; // tear down the notifications (C# DisconnectHandler); touches only the pimpl
        void host_content();
        void apply_title();
        void apply_frame();

        property_mapper_base* mapper_;
        i_maui_context* maui_context_ = nullptr;
        std::unique_ptr<window_platform> platform_view_;
        i_element* virtual_view_ = nullptr; // non-owning (the window owns the handler)
        i_window* window_view_ = nullptr;   // the same, narrowed to the i_window contract
    };
} // namespace maui::core
