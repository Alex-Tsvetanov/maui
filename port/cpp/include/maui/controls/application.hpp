#pragma once
// maui::controls::application  <=  Microsoft.Maui.Controls.Application (+ Microsoft.Maui.IApplication)
//
// The root of a running app: it owns the set of open windows and the one-time start hook. open_window adds
// a window, starts the app the first time (Window.Created → Application.SendStart), and activates it;
// close_window destroys + removes it. An application is an element, so each window is a logical child and
// the application's BindingContext inherits down to the windows (and on to their pages). Ported from
// Application.cs (the OpenWindow/CloseWindow + SendStart drive). Multi-window orchestration, themes,
// resume/sleep, persisted state, and the platform application object are out of scope (STATUS.md).

#include <functional>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class application : public element
    {
    public:
        // The open windows (IApplication.Windows). NON-owning — the caller owns the windows' lifetime.
        [[nodiscard]] const std::vector<window*>& windows() const
        {
            return windows_;
        }
        // The first-opened window, or null when none are open (a convenience for the common single-window app).
        [[nodiscard]] window* main_window() const
        {
            return windows_.empty() ? nullptr : windows_.front();
        }

        // IApplication.OpenWindow: add the window, start the app on the first open, then create + activate it
        // (so its page Appears + Loads). Opening an already-open window is a no-op.
        void open_window(window& value);
        // IApplication.CloseWindow: destroy the window (Deactivating/Unloading its page) and drop it.
        void close_window(window& value);

        // Fired once, when the application starts (the first window opens) — Application.OnStart's seam.
        maui::core::event<> started;

    protected:
        // Each open window is a logical child, so the application's BindingContext inherits down to it.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (window* const value : windows_)
            {
                visit(*value);
            }
        }
        // Application.OnStart override point (a subclass may create its main window here).
        virtual void on_start()
        {
        }

    private:
        void send_start(); // idempotent: raise `started` + call on_start once

        std::vector<window*> windows_; // NON-owning open windows (IApplication.Windows)
        bool started_ = false;
    };
} // namespace maui::controls
