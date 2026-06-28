#pragma once
// maui::ui::app  <=  the consumer-facing application base (hides maui::core::i_window from create_window()).
//
// Two hosting shapes:
//   - DEFAULT (recommended): call set_content(root) [+ set_title(...)] in the ctor. ui::app owns BOTH the
//     main window and the content root, declared in the correct teardown order — the window non-owningly
//     references its content (set_content + the window's menu/toolbar trackers subscribe into the page
//     tree), so the content must OUTLIVE the window. ui::app handles that once here, so the consumer
//     declares neither member and cannot get the order wrong (it was a real ASan-found use-after-free).
//   - CUSTOM: override main_window() to return your own window; you then own + order window<->content.
//
// Either way `app` seals create_window() (the framework's i_window* contract) onto main_window(), so a
// consumer never names an i_* type. Additive: maui::controls::application is unchanged. See
// port/cpp/docs/PUBLIC_API_DESIGN.md (§3-H).

#include <string>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/ui/view_ref.hpp"

namespace maui::ui
{
    class app : public maui::controls::application
    {
    public:
        // Host `root` as the main window's content. ui::app owns the window and the root (correct teardown
        // order), so the consumer holds neither as a member.
        void set_content(view_ref<maui::controls::content_page> root)
        {
            content_root_ = std::move(root);
            default_window_.set_content(content_root_.impl());
        }
        void set_title(std::string title)
        {
            default_window_.set_title(std::move(title));
        }

        // The application's main window — the ui::app-owned window by default; override to manage your own.
        [[nodiscard]] virtual maui::controls::window& main_window()
        {
            return default_window_;
        }

        // Sealed forwarder for IApplication.CreateWindow: hands the framework the i_window* from the concrete
        // main_window() (maui::controls::window derives maui::core::i_window).
        [[nodiscard]] maui::core::i_window* create_window() final
        {
            return &main_window();
        }

    protected:
        // content_root_ declared BEFORE default_window_ so default_window_ destructs FIRST, while the page it
        // references is still alive — the window<->content teardown ordering, handled once for every consumer.
        view_ref<maui::controls::content_page> content_root_;
        maui::controls::window default_window_;
    };
} // namespace maui::ui
