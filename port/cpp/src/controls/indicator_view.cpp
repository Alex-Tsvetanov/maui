// indicator_view — the ctor, the descriptors (with the ItemsSource → Count subscription), the
// items-source reset/recount machinery, and the default-handler self-registration. Ported from
// IndicatorView.cs (+ IndicatorViewExtensions for the handler-side count logic).

#include "maui/controls/indicator_view.hpp"

#include <climits>
#include <memory>

#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::controls
{
    indicator_view::indicator_view()
    {
        this->set_style_target_type<indicator_view>();
    }

    // IndicatorView.IndicatorsShapeProperty: default Circle.
    const maui::core::bindable_property<maui::controls::indicator_shape>& indicator_view::indicators_shape_property()
    {
        static const maui::core::bindable_property<maui::controls::indicator_shape> descriptor{
            "indicators_shape", maui::controls::indicator_shape::circle};
        return descriptor;
    }

    // IndicatorView.PositionProperty: default 0, TwoWay.
    const maui::core::bindable_property<int>& indicator_view::position_property()
    {
        static const maui::core::bindable_property<int> descriptor{
            "position", 0, {.default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // IndicatorView.CountProperty: default 0.
    const maui::core::bindable_property<int>& indicator_view::count_property()
    {
        static const maui::core::bindable_property<int> descriptor{"count", 0};
        return descriptor;
    }

    // IndicatorView.MaximumVisibleProperty: default int.MaxValue.
    const maui::core::bindable_property<int>& indicator_view::maximum_visible_property()
    {
        static const maui::core::bindable_property<int> descriptor{"maximum_visible", INT_MAX};
        return descriptor;
    }

    // IndicatorView.HideSingleProperty: default true.
    const maui::core::bindable_property<bool>& indicator_view::hide_single_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"hide_single", true};
        return descriptor;
    }

    // IndicatorView.IndicatorColorProperty: default Colors.LightGrey.
    const maui::core::bindable_property<maui::graphics::color>& indicator_view::indicator_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{
            "indicator_color", maui::graphics::colors::light_grey};
        return descriptor;
    }

    // IndicatorView.SelectedIndicatorColorProperty: default Colors.Black.
    const maui::core::bindable_property<maui::graphics::color>& indicator_view::selected_indicator_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"selected_indicator_color",
                                                                                     maui::graphics::colors::black};
        return descriptor;
    }

    // IndicatorView.IndicatorSizeProperty: default 6.0.
    const maui::core::bindable_property<double>& indicator_view::indicator_size_property()
    {
        static const maui::core::bindable_property<double> descriptor{"indicator_size", 6.0};
        return descriptor;
    }

    // IndicatorView.ItemsSourceProperty: default null; a change re-subscribes + recounts (the C#
    // ResetItemsSource → OnCollectionChanged → Count).
    const maui::core::bindable_property<std::shared_ptr<i_item_collection>>& indicator_view::items_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>> descriptor{
            "items_source",
            nullptr,
            {.property_changed = [](maui::core::bindable_object& bindable, const std::shared_ptr<i_item_collection>&,
                                    const std::shared_ptr<i_item_collection>&) {
                dynamic_cast<indicator_view&>(bindable).reset_items_source();
            }}};
        return descriptor;
    }

    // ResetItemsSource: drop the old subscription, subscribe to the new live collection (if any), and
    // recompute Count (the C# OnCollectionChanged(Reset) on assignment).
    void indicator_view::reset_items_source()
    {
        items_source_changed_ = maui::core::scoped_connection{}; // disconnect the old (§8)
        tracked_source_ = items_source_.get();                   // PIN the new source before subscribing
        if (tracked_source_)
        {
            if (auto* changed = tracked_source_->changed())
            {
                items_source_changed_ =
                    maui::core::connect_scoped(*changed, [this](const maui::core::collection_changed_args&) {
                        refresh_count_from_items_source();
                    });
            }
        }
        refresh_count_from_items_source();
    }

    // OnCollectionChanged: Count = collection.Count (0 when the source is null).
    void indicator_view::refresh_count_from_items_source()
    {
        set_count(tracked_source_ ? static_cast<int>(tracked_source_->count()) : 0);
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::indicator_view, maui::core::indicator_view_handler)
