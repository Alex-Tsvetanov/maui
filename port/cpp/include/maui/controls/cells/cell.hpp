#pragma once
// maui::controls::cell  <=  Microsoft.Maui.Controls.Cell
//
// The abstract base for every cell hosted in a table_view (or list_view). An Element subtype — cells
// carry data + the activation/lifecycle events, NOT a handler/native view of their own (a table/list
// realizes a cell into a native row via its own handler). Ported from
// src/Controls/src/Core/Cells/Cell.cs.
//
// Ported surface (the table-view-relevant subset; the obsolete C# control is faithfully modeled):
//   - DefaultCellHeight (40), Height (default -1, INPC on change of Height + RenderHeight),
//     IsEnabled (bindable, default true; a change re-raises HasContextActions),
//   - RenderHeight — the EFFECTIVE row height: walks the RealParent (a table_view or list_view) for
//     HasUnevenRows + RowHeight, else DefaultCellHeight (Cell.RenderHeight),
//   - Tapped + OnTapped (the activation seam — text_cell overrides to fire its command),
//     Appearing/Disappearing + SendAppearing/SendDisappearing + OnAppearing/OnDisappearing,
//   - ContextActions (an owned menu_item list) + HasContextActions (non-empty AND IsEnabled), with
//     the new context items parented to the cell + given its binding context (Cell.OnContextActionsChanged),
//   - ForceUpdateSize / ForceUpdateSizeRequested — rate-limited to once per outstanding request, and
//     ONLY when the parent table/list HasUnevenRows (Cell.ForceUpdateSize). DEVIATION: C# debounces on a
//     16 ms Task.Delay; the headless port fires synchronously once per outstanding request and resets the
//     queued flag immediately (no UI thread to await) — the observable "rate-limited to one call" holds.
//
// Out of scope (documented): IsContextActionsLegacyModeEnabled, the platform reuse hooks
// (ReusableCell/ConvertView), IElementConfiguration<Cell>, and the FlowDirection/Visual propagation
// controllers (the wider visual-tree propagation is element's job, already ported).

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/menu_item.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class cell : public element
    {
    public:
        // Cell.DefaultCellHeight.
        static constexpr int default_cell_height = 40;

        cell();
        ~cell() override;
        cell(const cell&) = delete;
        cell(cell&&) = delete;
        cell& operator=(const cell&) = delete;
        cell& operator=(cell&&) = delete;

        // Shared bindable-property descriptor (Cell.IsEnabledProperty: default true).
        static const maui::core::bindable_property<bool>& is_enabled_property();

        // ---- Height (Cell.Height; default -1, plain field-backed bindable surface) ----
        [[nodiscard]] double height() const
        {
            return height_;
        }
        void set_height(double value);

        // ---- IsEnabled (Cell.IsEnabled; bindable, default true) ----
        [[nodiscard]] bool is_enabled() const
        {
            return is_enabled_.get();
        }
        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }

        // Cell.RenderHeight — the effective rendered row height (walks the real parent for
        // HasUnevenRows + RowHeight; default_cell_height otherwise).
        [[nodiscard]] double render_height() const;

        // ---- ContextActions (Cell.ContextActions / HasContextActions) ----
        // The owned context-menu items; the cell parents each + flows its binding context down.
        [[nodiscard]] const std::vector<std::shared_ptr<menu_item>>& context_actions() const
        {
            return context_actions_;
        }
        void add_context_action(std::shared_ptr<menu_item> item);
        [[nodiscard]] bool has_context_actions() const
        {
            return !context_actions_.empty() && is_enabled();
        }

        // ---- activation + lifecycle events ----
        maui::core::event<> tapped;       // Cell.Tapped
        maui::core::event<> appearing;    // Cell.Appearing
        maui::core::event<> disappearing; // Cell.Disappearing
        // Cell.ForceUpdateSizeRequested (EditorBrowsableState.Never — the platform reuse seam).
        maui::core::event<> force_update_size_requested;

        // Cell.OnTapped — the activation seam. Virtual: text_cell overrides to execute its command.
        virtual void on_tapped();

        // Cell.SendAppearing / SendDisappearing (the platform realize/recycle seam — fires OnAppearing /
        // OnDisappearing). C# also notifies a list_view parent per-cell; that hook is deferred with the
        // rest of list_view (see the list_view deviation in STATUS).
        void send_appearing();
        void send_disappearing();

        // Cell.ForceUpdateSize — request a size refresh; rate-limited + gated on the parent's
        // HasUnevenRows (see header note).
        void force_update_size();

        // Cell.OnParentPropertyChanged on "RowHeight": the table/list parent's RowHeight changed, so
        // RenderHeight (computed from it) changed — fire RenderHeight's INPC. The table_view calls this
        // on every cell when its RowHeight changes (the port's stand-in for the C# parent subscription).
        void notify_render_height_changed()
        {
            on_property_changing("render_height");
            on_property_changed("render_height");
        }

    protected:
        // Cell.OnAppearing / OnDisappearing — raise the event (override to observe; CALL THE BASE).
        virtual void on_appearing();
        virtual void on_disappearing();

        // element hook: a context-action item is a logical child, so propagate the binding context to it.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        // Grants the IsEnabledProperty descriptor callback (a free lambda) access to fire the protected
        // change notification, exactly as C#'s static OnIsEnabledPropertyChanged lives inside the class.
        friend struct cell_descriptor_access;
        // Cell.OnIsEnabledPropertyChanged: OnPropertyChanged(nameof(HasContextActions)).
        void notify_has_context_actions_changed()
        {
            on_property_changed("has_context_actions");
        }

        double height_ = -1;               // Cell._height (Cell.Height default -1)
        bool force_update_queued_ = false; // Cell._nextCallToForceUpdateSizeQueued

        maui::core::property<bool> is_enabled_{*this, is_enabled_property()};
        std::vector<std::shared_ptr<menu_item>> context_actions_; // Cell.ContextActions (owned)
    };

    // The contract a cell's table/list parent satisfies so render_height can read its row metrics.
    // (Cell.RenderHeight casts RealParent to TableView/ListView; the reflection-free port dynamic_casts
    // to this shared interface, which both controls implement.)
    class i_cell_container
    {
    public:
        virtual ~i_cell_container() = default;
        [[nodiscard]] virtual bool has_uneven_rows() const = 0;
        [[nodiscard]] virtual int row_height() const = 0;

    protected:
        i_cell_container() = default;
        i_cell_container(const i_cell_container&) = default;
        i_cell_container(i_cell_container&&) = default;
        i_cell_container& operator=(const i_cell_container&) = default;
        i_cell_container& operator=(i_cell_container&&) = default;
    };
} // namespace maui::controls
