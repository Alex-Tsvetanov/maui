// maui::controls::window — the IWindow lifecycle drive + page hosting (window.hpp).
#include "maui/controls/window.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"

namespace maui::controls
{
    window::window(content_page& page)
    {
        set_content(page);
    }

    void window::set_content(element& page)
    {
        if (content_ == &page)
        {
            return;
        }
        if (content_ != nullptr && is_activated_)
        {
            detach_page(); // tear the old page off the active window first
        }
        content_ = &page;
        content_->set_inherited_binding_context(raw_binding_context()); // the page inherits the window context
        if (is_activated_)
        {
            attach_page();
        }
    }

    void window::send_created()
    {
        if (is_created_)
        {
            return; // C# throws "already created"; the port no-ops
        }
        is_created_ = true;
        created.raise();
    }

    void window::send_activated()
    {
        if (is_activated_)
        {
            return;
        }
        is_activated_ = true;
        activated.raise();
        if (content_ != nullptr)
        {
            attach_page(); // window now active -> page Loaded + Appearing
        }
    }

    void window::send_deactivated()
    {
        if (!is_activated_)
        {
            return;
        }
        is_activated_ = false;
        if (content_ != nullptr)
        {
            detach_page(); // page Disappearing + Unloaded
        }
        deactivated.raise();
    }

    void window::send_destroying()
    {
        if (is_activated_)
        {
            send_deactivated();
        }
        is_created_ = false;
        destroying.raise();
    }

    void window::attach_page()
    {
        content_->set_containing_window(this); // page (+ its subtree) is now in a window -> Loaded
        if (auto* page = dynamic_cast<content_page*>(content_))
        {
            page->send_appearing(); // window-rooted Appearing (idempotent via has_appeared_)
        }
    }

    void window::detach_page()
    {
        if (auto* page = dynamic_cast<content_page*>(content_))
        {
            page->send_disappearing();
        }
        content_->set_containing_window(nullptr); // page (+ subtree) leaves the window -> Unloaded
    }
} // namespace maui::controls
