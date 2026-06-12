#pragma once
// maui::controls::items_view  <=  Microsoft.Maui.Controls.ItemsView
//
// The abstract base of the templated-collection controls (structured → selectable → groupable →
// reorderable → collection_view): ItemsSource + ItemTemplate, EmptyView (+template), the scroll-bar
// visibilities, ItemsUpdatingScrollMode, the remaining-items threshold (event + the port's
// move_only_function command convention), ScrollTo, and the internal ItemsLayout slot
// StructuredItemsView exposes. Ported from src/Controls/src/Core/Items/ItemsView.cs.
//
// Shape notes (deviations documented):
//   - ItemsSource (C# IEnumerable) is the erased i_item_collection seam: typed setters wrap an
//     observable_collection<T> (live) or a std::vector<T> (snapshot); the grouped flavor passes an
//     item_collection<grouping_ptr> (see item_collection.hpp). Items cross as boxed_item.
//   - EmptyView (C# object) is a boxed_item: a boxed view (as_bindable) hosts directly, a boxed
//     string/value renders its text, exactly the C# View-vs-ToString split.
//   - RemainingItemsThresholdReachedCommand is the move_only_function member convention (the button
//     `command` precedent; no ICommand/CanExecute surface in the port) and runs AFTER the event, the
//     C# SendRemainingItemsThresholdReached order.
//   - The internal ItemsLayout defaults to a fresh vertical linear layout minted in the CTOR (C#'s
//     defaultValueCreator mints it on first read — same instance-per-view semantics, eager here);
//     it inherits this view's BindingContext (OnInternalItemsLayoutPropertyChanged +
//     OnBindingContextChanged, the ItemsViewTests.BindingContextPropagatesLayouts oracle).
//   - ScrollTo raises scroll_to_requested AND funnels the args through the handler's "scroll_to"
//     command (C# wires the event to the handler in ConnectHandler; the port collapses that
//     subscription into the command seam — the scroll_view RequestScrollTo precedent). DismissScroll
//     (no handler or no ItemsSource → no-op) is preserved.
//   - The OnMeasure screen-clamp lives in the handler's get_desired_size (the port's measure seam);
//     the C# 40x40 minimum SizeRequest has no port analog (documented).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_scrolled_event_args.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/property.hpp"
#include "maui/core/scroll_bar_visibility.hpp"

namespace maui::controls
{
    class items_view : public view<i_items_view>
    {
    public:
        // Shared descriptors (ItemsView.*Property).
        static const maui::core::bindable_property<boxed_item>& empty_view_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& empty_view_template_property();
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>>& items_source_property();
        static const maui::core::bindable_property<int>& remaining_items_threshold_property();
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility>&
        horizontal_scroll_bar_visibility_property();
        static const maui::core::bindable_property<maui::core::scroll_bar_visibility>&
        vertical_scroll_bar_visibility_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& item_template_property();
        static const maui::core::bindable_property<controls::items_updating_scroll_mode>&
        items_updating_scroll_mode_property();

        // ---- i_items_view ----
        [[nodiscard]] const std::shared_ptr<i_item_collection>& items_source() const override
        {
            return items_source_.get();
        }
        [[nodiscard]] const std::shared_ptr<data_template>& item_template() const override
        {
            return item_template_.get();
        }
        [[nodiscard]] const boxed_item& empty_view() const override
        {
            return empty_view_.get();
        }
        [[nodiscard]] const std::shared_ptr<data_template>& empty_view_template() const override
        {
            return empty_view_template_.get();
        }
        [[nodiscard]] controls::items_updating_scroll_mode items_updating_scroll_mode() const override
        {
            return items_updating_scroll_mode_.get();
        }
        [[nodiscard]] maui::core::scroll_bar_visibility horizontal_scroll_bar_visibility() const override
        {
            return horizontal_scroll_bar_visibility_.get();
        }
        [[nodiscard]] maui::core::scroll_bar_visibility vertical_scroll_bar_visibility() const override
        {
            return vertical_scroll_bar_visibility_.get();
        }
        [[nodiscard]] int remaining_items_threshold() const override
        {
            return remaining_items_threshold_.get();
        }

        // ---- ItemsSource (typed entries over the erased seam; header note) ----
        template <class TItem> void set_items_source(std::shared_ptr<maui::core::observable_collection<TItem>> items)
        {
            set_items_source(make_item_collection(std::move(items)));
        }
        template <class TItem> void set_items_source(std::vector<TItem> items)
        {
            set_items_source(make_item_collection(std::move(items)));
        }
        void set_items_source(std::shared_ptr<i_item_collection> items)
        {
            items_source_.set(std::move(items));
        }
        // C# ItemsSource = null.
        void clear_items_source()
        {
            items_source_.set(nullptr);
        }

        // ---- setters ----
        void set_item_template(std::shared_ptr<data_template> value)
        {
            item_template_.set(std::move(value));
        }
        void set_empty_view(boxed_item value)
        {
            empty_view_.set(std::move(value));
        }
        void set_empty_view_template(std::shared_ptr<data_template> value)
        {
            empty_view_template_.set(std::move(value));
        }
        void set_items_updating_scroll_mode(controls::items_updating_scroll_mode value)
        {
            items_updating_scroll_mode_.set(value);
        }
        void set_horizontal_scroll_bar_visibility(maui::core::scroll_bar_visibility value)
        {
            horizontal_scroll_bar_visibility_.set(value);
        }
        void set_vertical_scroll_bar_visibility(maui::core::scroll_bar_visibility value)
        {
            vertical_scroll_bar_visibility_.set(value);
        }
        // Values < -1 are ignored (the C# validateValue; -1 disables the threshold).
        void set_remaining_items_threshold(int value)
        {
            remaining_items_threshold_.set(value);
        }

        // ---- ScrollTo (the two C# overloads; header note) ----
        void scroll_to(int index, int group_index = -1,
                       controls::scroll_to_position position = controls::scroll_to_position::make_visible,
                       bool animate = true);
        void scroll_to(const boxed_item& item, const boxed_item& group = {},
                       controls::scroll_to_position position = controls::scroll_to_position::make_visible,
                       bool animate = true);

        // ---- inbound channels (i_items_view) ----
        void send_remaining_items_threshold_reached() override;
        void send_scrolled(const items_view_scrolled_event_args& args) override;

        // ---- events + the command convention (header note) ----
        maui::core::event<const scroll_to_request_event_args&> scroll_to_requested;
        maui::core::event<const items_view_scrolled_event_args&> scrolled;
        maui::core::event<> remaining_items_threshold_reached;
        maui::core::move_only_function<void()> remaining_items_threshold_reached_command;

    protected:
        // ItemsView is abstract (C#).
        items_view();

        // C# OnScrollToRequested / OnRemainingItemsThresholdReached / OnScrolled hooks.
        virtual void on_scroll_to_requested(const scroll_to_request_event_args& args);
        virtual void on_remaining_items_threshold_reached()
        {
        }
        virtual void on_scrolled(const items_view_scrolled_event_args& args)
        {
            (void)args;
        }

        // The internal ItemsLayout slot (ItemsView.InternalItemsLayout; StructuredItemsView makes it
        // public). Never null — defaults to a fresh vertical linear layout (header note).
        [[nodiscard]] const std::shared_ptr<controls::items_layout>& internal_items_layout() const
        {
            return internal_items_layout_;
        }
        void set_internal_items_layout(std::shared_ptr<controls::items_layout> value);

        // OnBindingContextChanged: the layout inherits this view's context.
        void on_binding_context_changed() override;

    private:
        // C# DismissScroll: no handler or no ItemsSource → the request is dropped.
        [[nodiscard]] bool dismiss_scroll() const;
        void request_scroll(const scroll_to_request_event_args& args);

        std::shared_ptr<controls::items_layout> internal_items_layout_;
        maui::core::property<boxed_item> empty_view_{*this, empty_view_property()};
        maui::core::property<std::shared_ptr<data_template>> empty_view_template_{*this,
                                                                                  empty_view_template_property()};
        maui::core::property<std::shared_ptr<i_item_collection>> items_source_{*this, items_source_property()};
        maui::core::property<int> remaining_items_threshold_{*this, remaining_items_threshold_property()};
        maui::core::property<maui::core::scroll_bar_visibility> horizontal_scroll_bar_visibility_{
            *this, horizontal_scroll_bar_visibility_property()};
        maui::core::property<maui::core::scroll_bar_visibility> vertical_scroll_bar_visibility_{
            *this, vertical_scroll_bar_visibility_property()};
        maui::core::property<std::shared_ptr<data_template>> item_template_{*this, item_template_property()};
        maui::core::property<controls::items_updating_scroll_mode> items_updating_scroll_mode_{
            *this, items_updating_scroll_mode_property()};
    };
} // namespace maui::controls
