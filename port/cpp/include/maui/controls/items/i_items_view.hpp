#pragma once
// maui::controls::i_items_view  <=  the virtual-view face of Microsoft.Maui.Controls.ItemsView
//
// C# has NO Core interface for the Items layer — ItemsViewHandler<TItemsView> is typed directly on
// the Controls class. The port keeps the handler seam honest with this controls-layer interface
// carrying the ItemsView-level surface the mapper reads plus the inbound send_* channels the platform
// drives; the StructuredItemsView/SelectableItemsView/... extras are reached by dynamic_cast to the
// concrete classes inside the map functions (the analog of C#'s `where TItemsView : ItemsView`
// typing — both sides live in the controls layer, like Controls.Handlers.Items does).

#include <memory>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_scrolled_event_args.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/scroll_bar_visibility.hpp"

namespace maui::controls
{
    class data_template;

    class i_items_view : public maui::core::i_view
    {
    public:
        // The erased ItemsSource (null = C# null → the empty source / EmptyView path).
        [[nodiscard]] virtual const std::shared_ptr<i_item_collection>& items_source() const = 0;
        [[nodiscard]] virtual const std::shared_ptr<data_template>& item_template() const = 0;
        [[nodiscard]] virtual const boxed_item& empty_view() const = 0;
        [[nodiscard]] virtual const std::shared_ptr<data_template>& empty_view_template() const = 0;
        [[nodiscard]] virtual controls::items_updating_scroll_mode items_updating_scroll_mode() const = 0;
        [[nodiscard]] virtual maui::core::scroll_bar_visibility horizontal_scroll_bar_visibility() const = 0;
        [[nodiscard]] virtual maui::core::scroll_bar_visibility vertical_scroll_bar_visibility() const = 0;
        [[nodiscard]] virtual int remaining_items_threshold() const = 0;

        // Inbound channels (platform → virtual): the scrolled report and the threshold trip.
        virtual void send_scrolled(const items_view_scrolled_event_args& args) = 0;
        virtual void send_remaining_items_threshold_reached() = 0;
    };
} // namespace maui::controls
