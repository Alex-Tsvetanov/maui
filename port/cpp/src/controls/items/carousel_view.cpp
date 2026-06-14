// carousel_view — the ctor (default snap layout), the descriptors with the C# Position/CurrentItem
// choreography, and the default-handler self-registration (CollectionViewHandler reuse). Ported from
// CarouselView.cs.

#include "maui/controls/items/carousel_view.hpp"

#include <memory>
#include <utility>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    carousel_view::carousel_view()
    {
        this->set_style_target_type<carousel_view>();
        // CarouselView() ctor: a horizontal LinearItemsLayout with MandatorySingle / Center snap (the
        // carousel "snaps one item into place" default — the ItemsLayoutProperty defaultValueCreator
        // mints CreateCarouselHorizontalDefault, reproduced here so the snap mirrors the W3-29 handler).
        auto layout = std::make_shared<linear_items_layout>(items_layout_orientation::horizontal);
        layout->set_snap_points_type(snap_points_type::mandatory_single);
        layout->set_snap_points_alignment(snap_points_alignment::center);
        set_items_layout(std::move(layout));
    }

    // CarouselView.LoopProperty: default true, OneTime.
    const maui::core::bindable_property<bool>& carousel_view::loop_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "loop", true, {.default_binding_mode = maui::core::binding_mode::one_time}};
        return descriptor;
    }

    // CarouselView.PeekAreaInsetsProperty: default 0 thickness.
    const maui::core::bindable_property<maui::core::thickness>& carousel_view::peek_area_insets_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"peek_area_insets"};
        return descriptor;
    }

    // CarouselView.IsBounceEnabledProperty: default true.
    const maui::core::bindable_property<bool>& carousel_view::is_bounce_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_bounce_enabled", true};
        return descriptor;
    }

    // CarouselView.IsSwipeEnabledProperty: default true.
    const maui::core::bindable_property<bool>& carousel_view::is_swipe_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_swipe_enabled", true};
        return descriptor;
    }

    // CarouselView.IsScrollAnimatedProperty: default true.
    const maui::core::bindable_property<bool>& carousel_view::is_scroll_animated_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_scroll_animated", true};
        return descriptor;
    }

    // CarouselView.CurrentItemProperty: default null, TwoWay, CurrentItemPropertyChanged.
    const maui::core::bindable_property<boxed_item>& carousel_view::current_item_property()
    {
        static const maui::core::bindable_property<boxed_item> descriptor{
            "current_item",
            boxed_item{},
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const boxed_item& old_value, const boxed_item& new_value) {
                     dynamic_cast<carousel_view&>(bindable).raise_current_item_changed(old_value, new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // CarouselView.PositionProperty: default 0, TwoWay, PositionPropertyChanged.
    const maui::core::bindable_property<int>& carousel_view::position_property()
    {
        static const maui::core::bindable_property<int> descriptor{
            "position",
            0,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const int& old_value, const int& new_value) {
                     dynamic_cast<carousel_view&>(bindable).raise_position_changed(old_value, new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // PositionPropertyChanged: command → event → the OnPositionChanged hook (the C# order). The renderer
    // half (scroll the native carousel to the new position) reuses the items ScrollTo command — like
    // CarouselViewHandler2.MapPosition → ScrollToPosition, centered (the carousel snap alignment).
    // ScrollTo is a no-op without a handler or items source (items_view::dismiss_scroll), matching C#.
    void carousel_view::raise_position_changed(int old_value, int new_value)
    {
        if (position_changed_command)
        {
            position_changed_command();
        }
        const position_changed_event_args args{.previous_position = old_value, .current_position = new_value};
        position_changed.raise(args);
        on_position_changed(args);
        scroll_to(new_value, -1, scroll_to_position::center, animate_position_changes());
    }

    // CurrentItemPropertyChanged: command → event → the OnCurrentItemChanged hook (the C# order).
    void carousel_view::raise_current_item_changed(const boxed_item& old_value, const boxed_item& new_value)
    {
        if (current_item_changed_command)
        {
            current_item_changed_command();
        }
        const current_item_changed_event_args args{.previous_item = old_value, .current_item = new_value};
        current_item_changed.raise(args);
        on_current_item_changed(args);
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6) — the carousel reuses the collection_view
// virtualization handler (the iOS compositional path + the headless simulator + scroll_to). This TU is
// always linked (the carousel descriptors are referenced by every user of the control).
MAUI_REGISTER_HANDLER(maui::controls::carousel_view, maui::controls::collection_view_handler)
