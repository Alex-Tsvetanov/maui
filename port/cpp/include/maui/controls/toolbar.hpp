#pragma once
// maui::controls::toolbar  <=  Microsoft.Maui.Controls.Toolbar (+ the NavigationPageToolbar aggregate)
//
// The window-level toolbar chrome: the priority-sorted toolbar items the window chrome host
// materializes (an NSToolbar on AppKit; the iOS items surface through the navigation bar instead —
// see navigation_page_handler). Ported from src/Controls/src/Core/Toolbar/Toolbar.cs, scoped to the
// surface this cut maps: ToolbarItems / Title / IsVisible / BackButtonVisible. C#'s Toolbar is an
// INotifyPropertyChanged chrome object (NOT a BindableObject) whose SetProperty pokes ITS handler
// (a separate ToolbarHandler); the port collapses that to a notify callback the owning window wires
// to its own window_handler ("toolbar" map) — an NSToolbar is intrinsically window-attached, so a
// separate toolbar handler would host nothing of its own (documented collapse, the same shape as the
// navigation bar living in navigation_page_handler).
//
// The aggregation side of C#'s NavigationPageToolbar (its ToolbarTracker → ToolbarItems sync) is the
// window's wiring: controls::window owns the toolbar_tracker and pushes tracker.toolbar_items() into
// set_toolbar_items here on every collection change.

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/toolbar_item.hpp"
#include "maui/core/i_toolbar.hpp"
#include "maui/core/i_toolbar_item.hpp"

namespace maui::controls
{
    class element;

    class toolbar : public maui::core::i_toolbar
    {
    public:
        // C# Toolbar(Maui.IElement parent) — the element this chrome belongs to (the window). Non-owning.
        explicit toolbar(element* parent = nullptr) : parent_(parent)
        {
        }

        [[nodiscard]] element* parent() const
        {
            return parent_;
        }

        // The handler-notification seam (C# SetProperty → Handler?.UpdateValue(propertyName)): the
        // owning window wires this to update_handler_value, so any chrome change re-runs the window
        // handler's "toolbar" map.
        void set_notify(std::function<void(std::string_view property)> value)
        {
            notify_ = std::move(value);
        }

        // C# Toolbar.ToolbarItems (the tracker-sourced aggregate; non-owning pointers).
        [[nodiscard]] const std::vector<toolbar_item*>& toolbar_items() const
        {
            return items_;
        }
        void set_toolbar_items(std::vector<toolbar_item*> value)
        {
            if (items_ == value)
            {
                return;
            }
            items_ = std::move(value);
            notify("toolbar_items");
        }

        void set_title(std::string value)
        {
            if (title_ == value)
            {
                return;
            }
            title_ = std::move(value);
            notify("title");
        }

        void set_is_visible(bool value)
        {
            if (is_visible_ == value)
            {
                return;
            }
            is_visible_ = value;
            notify("is_visible");
        }

        void set_back_button_visible(bool value)
        {
            if (back_button_visible_ == value)
            {
                return;
            }
            back_button_visible_ = value;
            notify("back_button_visible");
        }

        // ---- i_toolbar ----
        [[nodiscard]] bool back_button_visible() const override
        {
            return back_button_visible_;
        }
        [[nodiscard]] bool is_visible() const override
        {
            return is_visible_;
        }
        [[nodiscard]] std::string_view title() const override
        {
            return title_;
        }
        [[nodiscard]] std::size_t item_count() const override
        {
            return items_.size();
        }
        [[nodiscard]] maui::core::i_toolbar_item* item_at(std::size_t index) const override
        {
            return index < items_.size() ? items_[index] : nullptr;
        }

    private:
        void notify(std::string_view property)
        {
            if (notify_)
            {
                notify_(property);
            }
        }

        element* parent_ = nullptr;        // non-owning back-ref (C# Toolbar.Parent)
        std::vector<toolbar_item*> items_; // non-owning (the pages own the items)
        std::string title_;
        bool is_visible_ = false; // C# Toolbar._isVisible defaults false
        bool back_button_visible_ = false;
        std::function<void(std::string_view)> notify_;
    };
} // namespace maui::controls
