#pragma once
// maui::controls::shell_item (+ flyout_item, tab_bar)  <=  Microsoft.Maui.Controls.ShellItem /
// FlyoutItem / TabBar
//
// A top-level shell node grouping shell_sections. Ported from ShellItem.cs:
//   - items are OWNED shared_ptr<shell_section> children; current_item auto-selects the first
//     visible child on add and re-selects (or clears) when the current one is removed;
//   - create_from_shell_section is the explicit `implicit operator ShellItem(ShellSection)`: an
//     already-parented section returns (and selects into) its existing item; a fresh section is
//     wrapped (a `tab` becomes a tab_bar, like C#) with the IMPL_ route and a live Title/Icon/
//     FlyoutIcon sync standing in for the C# bindings;
//   - the CurrentItem changed flow (OnCurrentItemChanged) fires the Appearing/Disappearing pair and
//     updates the shell's CurrentState (ShellSectionChanged) when this item is visible.
// flyout_item / tab_bar are the alias subclasses, declared here like the C# file does. The
// MenuShellItem / platform-configuration surfaces are out of scope (no MenuItem flyout this unit).

#include <functional>
#include <memory>
#include <vector>

#include "maui/controls/shell/shell_group_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class shell;

    class shell_item : public shell_group_item
    {
    public:
        shell_item() = default;
        ~shell_item() override;
        shell_item(const shell_item&) = delete;
        shell_item(shell_item&&) = delete;
        shell_item& operator=(const shell_item&) = delete;
        shell_item& operator=(shell_item&&) = delete;

        // ---- Items (ShellItem.Items — owned children) ----
        [[nodiscard]] const std::vector<std::shared_ptr<shell_section>>& items() const
        {
            return items_;
        }
        void add(std::shared_ptr<shell_section> section);
        // Adopt-and-add conveniences (the C# implicit conversions at the collection seam).
        std::shared_ptr<shell_section> add(std::shared_ptr<shell_content> content);
        std::shared_ptr<shell_section> add(content_page& page);
        void remove(const shell_section& section);
        // IShellItemController.GetItems — the visible (IsVisible) children, point-in-time.
        [[nodiscard]] std::vector<shell_section*> visible_items() const;

        // ---- CurrentItem ----
        [[nodiscard]] shell_section* current_item() const
        {
            return current_item_;
        }
        void set_current_item(shell_section* value);

        // ShellItem.IsVisibleItem: this is the shell's current item.
        [[nodiscard]] bool is_visible_item() const;

        // The explicit `implicit operator ShellItem(ShellSection)` (CreateFromShellSection).
        [[nodiscard]] static std::shared_ptr<shell_item> create_from_shell_section(
            std::shared_ptr<shell_section> section);

        // SendStructureChanged: a visible item's structural change reaches the shell.
        void send_structure_changed();

        // ---- lifecycle fan-out (ShellItem.SendAppearing/SendDisappearing) ----
        void send_appearing() override;
        void send_disappearing() override;

        [[nodiscard]] shell* containing_shell() const;

    protected:
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        std::vector<std::shared_ptr<shell_section>> items_; // OWNED children
        shell_section* current_item_ = nullptr;             // borrowed view of one items_ entry
        // The CreateFromShellSection live sync (§8 ordering: declared after items_; reset on remove).
        shell_section* wrapper_source_ = nullptr;
        maui::core::scoped_connection wrapper_sync_token_;
    };

    // C# FlyoutItem — a ShellItem the flyout shows by default.
    class flyout_item : public shell_item
    {
    };

    // C# TabBar — a ShellItem presented as the bottom tab bar (flyout hidden).
    class tab_bar : public shell_item
    {
    };
} // namespace maui::controls
