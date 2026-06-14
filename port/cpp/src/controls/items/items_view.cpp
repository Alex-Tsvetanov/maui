// items_view — descriptors + ScrollTo / threshold / ItemsLayout-context machinery (ItemsView.cs).

#include "maui/controls/items/items_view.hpp"

#include <any>
#include <memory>
#include <utility>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_scrolled_event_args.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/scroll_bar_visibility.hpp"

namespace maui::controls
{
    items_view::items_view() : internal_items_layout_(linear_items_layout::create_vertical_default())
    {
    }

    const maui::core::bindable_property<boxed_item>& items_view::empty_view_property()
    {
        static const maui::core::bindable_property<boxed_item> descriptor{"empty_view"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& items_view::empty_view_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"empty_view_template"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<i_item_collection>>& items_view::items_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>> descriptor{"items_source"};
        return descriptor;
    }

    // ItemsView.RemainingItemsThresholdProperty: default -1, validateValue >= -1 (invalid sets ignored).
    const maui::core::bindable_property<int>& items_view::remaining_items_threshold_property()
    {
        static const maui::core::bindable_property<int> descriptor{
            "remaining_items_threshold", -1, {.validate_value = [](maui::core::bindable_object&, const int& value) {
                return value >= -1;
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::scroll_bar_visibility>& items_view::
        horizontal_scroll_bar_visibility_property()
    {
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility> descriptor{
            "horizontal_scroll_bar_visibility", maui::core::scroll_bar_visibility::default_};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::scroll_bar_visibility>& items_view::
        vertical_scroll_bar_visibility_property()
    {
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility> descriptor{
            "vertical_scroll_bar_visibility", maui::core::scroll_bar_visibility::default_};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& items_view::item_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"item_template"};
        return descriptor;
    }

    const maui::core::bindable_property<items_updating_scroll_mode>& items_view::items_updating_scroll_mode_property()
    {
        static const maui::core::bindable_property<controls::items_updating_scroll_mode> descriptor{
            "items_updating_scroll_mode", controls::items_updating_scroll_mode::keep_items_in_view};
        return descriptor;
    }

    // ---- ScrollTo ----

    void items_view::scroll_to(int index, int group_index, controls::scroll_to_position position, bool animate)
    {
        if (dismiss_scroll())
        {
            return;
        }
        request_scroll(scroll_to_request_event_args::for_position(index, group_index, position, animate));
    }

    void items_view::scroll_to(const boxed_item& item, const boxed_item& group, controls::scroll_to_position position,
                               bool animate)
    {
        if (dismiss_scroll())
        {
            return;
        }
        request_scroll(scroll_to_request_event_args::for_element(item, group, position, animate));
    }

    bool items_view::dismiss_scroll() const
    {
        return handler() == nullptr || items_source() == nullptr;
    }

    void items_view::request_scroll(const scroll_to_request_event_args& args)
    {
        on_scroll_to_requested(args);
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("scroll_to", std::any{args});
        }
    }

    void items_view::on_scroll_to_requested(const scroll_to_request_event_args& args)
    {
        scroll_to_requested.raise(args);
    }

    // ---- the remaining-items threshold + scrolled channels ----

    // SendRemainingItemsThresholdReached: event → command → the OnRemainingItemsThresholdReached hook.
    void items_view::send_remaining_items_threshold_reached()
    {
        remaining_items_threshold_reached.raise();
        if (remaining_items_threshold_reached_command)
        {
            remaining_items_threshold_reached_command();
        }
        on_remaining_items_threshold_reached();
    }

    void items_view::send_scrolled(const items_view_scrolled_event_args& args)
    {
        scrolled.raise(args);
        on_scrolled(args);
    }

    // ---- the internal ItemsLayout slot (header note) ----

    void items_view::set_internal_items_layout(std::shared_ptr<controls::items_layout> value)
    {
        if (internal_items_layout_ == value)
        {
            return;
        }
        // OnInternalItemsLayoutPropertyChanged: the old layout loses the inherited context, the new
        // one inherits this view's.
        if (internal_items_layout_)
        {
            internal_items_layout_->set_inherited_binding_context({});
        }
        internal_items_layout_ = std::move(value);
        if (internal_items_layout_)
        {
            internal_items_layout_->set_inherited_binding_context(raw_binding_context());
        }
        on_property_changed("items_layout");
    }

    void items_view::on_binding_context_changed()
    {
        view<i_items_view>::on_binding_context_changed();
        if (internal_items_layout_)
        {
            internal_items_layout_->set_inherited_binding_context(raw_binding_context());
        }
    }
} // namespace maui::controls
