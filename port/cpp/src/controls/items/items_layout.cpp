// items_layout — the shared snap-point descriptors (ItemsLayout.SnapPointsAlignmentProperty /
// SnapPointsTypeProperty: Start / None defaults).

#include "maui/controls/items/items_layout.hpp"

#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<snap_points_alignment>& items_layout::snap_points_alignment_property()
    {
        static const maui::core::bindable_property<controls::snap_points_alignment> descriptor{
            "snap_points_alignment", controls::snap_points_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<snap_points_type>& items_layout::snap_points_type_property()
    {
        static const maui::core::bindable_property<controls::snap_points_type> descriptor{
            "snap_points_type", controls::snap_points_type::none};
        return descriptor;
    }
} // namespace maui::controls
