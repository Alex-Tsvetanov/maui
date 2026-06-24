#pragma once
// maui::samples::application_control_page — ports ApplicationControlPage.xaml (+ .xaml.cs).
//
// The C# ApplicationControlPage is a BasePage (Title "App Control") with a headline label "Quits the
// application" and a centered "Terminate Application" button whose Clicked handler calls
// Application.Current!.Quit(). The port's application surface (maui::controls::application /
// maui::core::i_application) models OpenWindow / CloseWindow / Windows / theming but has NO Quit (the
// platform application object — and process termination — is out of scope, STATUS.md). The closest
// in-surface analog of Quit() is closing every open window (Application.CloseWindow over Windows), which
// this page exercises, alongside a readout of the live application state (window count + whether a main
// page/window is set) and buttons exercising the rest of the i_application window-management surface.
//
// Mapped from ApplicationControlPage.xaml.cs + generalized over the i_application surface:
//   - "Terminate Application" → close every open window  (the Quit() stand-in: Application.CloseWindow ×N)
//   - "Open Window"           → application.open_window(extra window)   (IApplication.OpenWindow)
//   - "Close Window"          → application.close_window(extra window)  (IApplication.CloseWindow)
// The readout echoes IApplication.Windows.Count, whether a main_window() is set, and that window's hosted
// page title (the "main page set" state) — a live view of the application state the buttons mutate.
//
// The page OWNS its visual tree (the sample_app pattern): the headline + readout labels, the three
// buttons, the stack, and the gallery content_page (page()). The application it drives is the HOSTING
// app's application (captured in on_mounted from maui_app::application()); the extra window it
// opens/closes is owned here (a member) so it outlives the application's NON-owning windows() list. It is
// backend-agnostic; the generic mount (app_host.hpp) attaches every view's handler and hosts the ctor-built tree.

#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class application_control_page
    {
    public:
        application_control_page()
        {
            page_.set_title("App Control"); // C# Title="App Control"
            stack_.set_spacing(12);

            headline_.set_text("Quits the application"); // C# headline label (Style=Headline)

            terminate_button_.set_text("Terminate Application"); // C# "Terminate Application"
            terminate_button_.clicked.connect([this] { terminate(); });

            open_button_.set_text("Open Window");
            open_button_.clicked.connect([this] { open_extra_window(); });

            close_button_.set_text("Close Window");
            close_button_.clicked.connect([this] { close_extra_window(); });

            readout_.set_text("Application: not yet hosted");

            // The extra window's hosted page — a simple content_page so the window has content to host.
            extra_page_.set_title("Extra window");

            stack_.add(headline_);
            stack_.add(terminate_button_);
            stack_.add(open_button_);
            stack_.add(close_button_);
            stack_.add(readout_);
            page_.set_content(stack_);
            // note: C# Application.Current!.Quit() terminates the process; the port has no Quit (the
            // platform application object is out of scope, STATUS.md). "Terminate Application" closes every
            // open window — the closest in-surface analog (Application.CloseWindow over Windows).
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // POST-MOUNT hook (gallery_host.hpp gallery_post_mount): run AFTER the generic mount attaches every
        // handler + builds the native tree. Capture the hosting application so the buttons can drive the
        // i_application window-management surface, then refresh the readout from the live application state.
        // All per-control attach + re-host plumbing is now the generic mount's job.
        void on_mounted(maui::hosting::maui_app& app)
        {
            app_ = &app; // the hosting maui_app (its open_window/close_window route the lifecycle)
            application_ = app.application().get(); // the minted controls::application (may be null if unconfigured)
            // The generic mount opens the window BEFORE on_mounted runs, so refresh() already reads the real
            // window count; also re-run on `started` so a later open/close keeps the readout live.
            if (application_ != nullptr)
            {
                application_->started.connect([this] { refresh(); });
            }
            refresh();
        }

        // ---- owned controls, exposed for the hosting main's bottom-up attachment + tests ----
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& headline()
        {
            return headline_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::button& terminate_button()
        {
            return terminate_button_;
        }
        [[nodiscard]] maui::controls::button& open_button()
        {
            return open_button_;
        }
        [[nodiscard]] maui::controls::button& close_button()
        {
            return close_button_;
        }
        [[nodiscard]] maui::controls::window& extra_window()
        {
            return extra_window_;
        }

    private:
        // C# OnTerminateClicked → Application.Current!.Quit(): the port's stand-in closes every open
        // window (Application.CloseWindow over a snapshot of Windows — closing mutates the list).
        void terminate()
        {
            if (application_ == nullptr)
            {
                return;
            }
            // Snapshot the concrete windows (close_window mutates windows_typed()).
            const std::vector<maui::controls::window*> open = application_->windows_typed();
            for (maui::controls::window* const win : open)
            {
                if (app_ != nullptr)
                {
                    app_->close_window(*win); // route through the hosting maui_app (unbridges the lifecycle)
                }
                else
                {
                    application_->close_window(*win);
                }
            }
            refresh();
        }

        // IApplication.OpenWindow: open the page-owned extra window (no-op if already open).
        void open_extra_window()
        {
            if (application_ == nullptr || extra_open_)
            {
                refresh();
                return;
            }
            extra_window_.set_content(extra_page_);
            if (app_ != nullptr)
            {
                app_->open_window(extra_window_); // hosting door: bridges the lifecycle, then opens
            }
            else
            {
                application_->open_window(extra_window_);
            }
            extra_open_ = true;
            refresh();
        }

        // IApplication.CloseWindow: close the page-owned extra window (no-op if not open).
        void close_extra_window()
        {
            if (application_ == nullptr || !extra_open_)
            {
                refresh();
                return;
            }
            if (app_ != nullptr)
            {
                app_->close_window(extra_window_);
            }
            else
            {
                application_->close_window(extra_window_);
            }
            extra_open_ = false;
            refresh();
        }

        // Echo the live application state into the readout: the open-window count, whether a main window
        // is set, and that window's hosted page title (the "main page set" state).
        void refresh()
        {
            if (application_ == nullptr)
            {
                readout_.set_text("Application: not configured (no application minted)");
                return;
            }
            const std::size_t window_count = application_->windows().size();
            std::string text = "Windows open: " + std::to_string(window_count);
            maui::controls::window* const main = application_->main_window();
            if (main != nullptr)
            {
                text += "  |  main window: ";
                text += std::string(main->title().empty() ? "(untitled)" : main->title());
                maui::controls::element* const content = main->content_element();
                text += content != nullptr ? "  |  main page: set" : "  |  main page: none";
            }
            else
            {
                text += "  |  main window: none";
            }
            readout_.set_text(text);
        }

        // ---- the gallery's own visual tree ----
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label headline_;
        maui::controls::button terminate_button_;
        maui::controls::button open_button_;
        maui::controls::button close_button_;
        maui::controls::label readout_;

        // ---- the i_application surface under exercise (captured in on_mounted) + the page-owned
        // extra window it opens/closes (NON-owning windows() list, so the window + its page are members) ----
        maui::hosting::maui_app* app_ = nullptr;             // the hosting maui_app (open/close door)
        maui::controls::application* application_ = nullptr; // the minted application (its windows()/theming)
        maui::controls::window extra_window_;
        maui::controls::content_page extra_page_;
        bool extra_open_ = false;
    };
} // namespace maui::samples
