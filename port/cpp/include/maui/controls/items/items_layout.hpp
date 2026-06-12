#pragma once
// maui::controls::items_layout  <=  Microsoft.Maui.Controls.ItemsLayout (+ IItemsLayout)
//
// The abstract item-arrangement model for collection (and later carousel) views: an orientation
// fixed at construction plus the bindable snap-point properties. A value/model type only — the
// arrangement math lives with the consumer (the headless fake-viewport simulator now, the native
// layouts in wave 3). C#'s IItemsLayout is a bare INotifyPropertyChanged marker; the port's contract
// IS this base class (documented collapse). The internal ItemsUpdatingScrollMode pass-through lives
// on the handler's platform mirror instead of here (the C# member is internal plumbing to the native
// layout, which the port reaches directly).

#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class items_layout : public maui::core::bindable_object
    {
    public:
        // Shared descriptors (ItemsLayout.SnapPoints*Property).
        static const maui::core::bindable_property<controls::snap_points_alignment>& snap_points_alignment_property();
        static const maui::core::bindable_property<controls::snap_points_type>& snap_points_type_property();

        // ItemsLayout.Orientation (get-only; fixed by the ctor).
        [[nodiscard]] items_layout_orientation orientation() const
        {
            return orientation_;
        }

        [[nodiscard]] controls::snap_points_alignment snap_points_alignment() const
        {
            return snap_points_alignment_.get();
        }
        void set_snap_points_alignment(controls::snap_points_alignment value)
        {
            snap_points_alignment_.set(value);
        }

        [[nodiscard]] controls::snap_points_type snap_points_type() const
        {
            return snap_points_type_.get();
        }
        void set_snap_points_type(controls::snap_points_type value)
        {
            snap_points_type_.set(value);
        }

    protected:
        explicit items_layout(items_layout_orientation orientation) : orientation_(orientation)
        {
        }

    private:
        items_layout_orientation orientation_;
        maui::core::property<controls::snap_points_alignment> snap_points_alignment_{*this,
                                                                                     snap_points_alignment_property()};
        maui::core::property<controls::snap_points_type> snap_points_type_{*this, snap_points_type_property()};
    };
} // namespace maui::controls
