#pragma once
// maui::controls::reorderable_items_view  <=  Microsoft.Maui.Controls.ReorderableItemsView
//
// Adds the reorder surface over groupable_items_view: CanReorderItems / CanMixGroups plus the
// ReorderCompleted event the platform fires through send_reorder_completed once a user drag lands.
// The drag itself is native (wave 3); the headless simulator drives the same channel.

#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class reorderable_items_view : public groupable_items_view
    {
    public:
        reorderable_items_view() = default;

        static const maui::core::bindable_property<bool>& can_mix_groups_property();
        static const maui::core::bindable_property<bool>& can_reorder_items_property();

        [[nodiscard]] bool can_mix_groups() const
        {
            return can_mix_groups_.get();
        }
        void set_can_mix_groups(bool value)
        {
            can_mix_groups_.set(value);
        }

        [[nodiscard]] bool can_reorder_items() const
        {
            return can_reorder_items_.get();
        }
        void set_can_reorder_items(bool value)
        {
            can_reorder_items_.set(value);
        }

        // ReorderCompleted + the platform channel (C# SendReorderCompleted).
        maui::core::event<> reorder_completed;
        void send_reorder_completed()
        {
            reorder_completed.raise();
        }

    private:
        maui::core::property<bool> can_mix_groups_{*this, can_mix_groups_property()};
        maui::core::property<bool> can_reorder_items_{*this, can_reorder_items_property()};
    };
} // namespace maui::controls
