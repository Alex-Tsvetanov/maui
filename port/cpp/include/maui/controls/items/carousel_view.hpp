#pragma once
// maui::controls::carousel_view  <=  Microsoft.Maui.Controls.CarouselView
//
// A scrollable items view where each item "snaps" into place — a horizontal/vertical carousel with
// looping and position tracking. Ported from src/Controls/src/Core/Items/CarouselView.cs.
//
// Shape notes (deviations documented):
//   - C# CarouselView derives ItemsView directly but defines its OWN ItemsLayoutProperty
//     (a LinearItemsLayout, defaulting to a horizontal Mandatory-Single / Center snap layout). The
//     port derives structured_items_view so it reuses the existing ItemsLayout face + the
//     collection_view_handler virtualization seam (the shared fake-viewport simulator + the iOS
//     compositional native path from W3-29) — exactly the reuse C#'s CarouselViewHandler2 :
//     ItemsViewHandler2<CarouselView> gets. The ctor installs the same horizontal MandatorySingle/
//     Center default layout the C# ctor builds (documented collapse).
//   - Position / CurrentItem are TwoWay bindables with the C# PositionPropertyChanged /
//     CurrentItemPropertyChanged choreography: the change fires the *Command (when CanExecute — the
//     port's command convention has no CanExecute, so it always runs), THEN the *Changed event, THEN
//     the protected On*Changed hook (verbatim C# order).
//   - CurrentItemChangedCommand / PositionChangedCommand are the move_only_function member convention
//     (the button `command` precedent) — no ICommand/CanExecute surface in the port.
//   - IsDragging / IsScrolling are renderer-set state (C# read-only key / EditorBrowsable.Never);
//     set_is_dragging mirrors SetIsDragging. VisibleViews (an ObservableCollection<View> the renderer
//     fills) has no headless analog and is OMITTED (documented — no virtualization realizes real
//     child views in the port's simulator).
//   - The handler registered is collection_view_handler (the carousel reuses the items virtualization
//     wholesale). The carousel knobs the W3-29 collection handler does not natively own
//     (PeekAreaInsets / IsSwipeEnabled / IsBounceEnabled) are control-side state pushed to the iOS
//     compositional layout where the native carousel handler reads them (documented).

#include <memory>
#include <utility>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    // CurrentItemChangedEventArgs (old, new) — the boxed item pair.
    struct current_item_changed_event_args
    {
        boxed_item previous_item;
        boxed_item current_item;
    };

    // PositionChangedEventArgs (old, new).
    struct position_changed_event_args
    {
        int previous_position = 0;
        int current_position = 0;
    };

    class carousel_view : public structured_items_view
    {
    public:
        carousel_view();

        // Shared descriptors (CarouselView.*Property).
        static const maui::core::bindable_property<bool>& loop_property();
        static const maui::core::bindable_property<maui::core::thickness>& peek_area_insets_property();
        static const maui::core::bindable_property<bool>& is_bounce_enabled_property();
        static const maui::core::bindable_property<bool>& is_swipe_enabled_property();
        static const maui::core::bindable_property<bool>& is_scroll_animated_property();
        static const maui::core::bindable_property<boxed_item>& current_item_property();
        static const maui::core::bindable_property<int>& position_property();

        // ---- Loop (OneTime in C#; default true) ----
        [[nodiscard]] bool loop() const
        {
            return loop_.get();
        }
        void set_loop(bool value)
        {
            loop_.set(value);
        }

        // ---- PeekAreaInsets ----
        [[nodiscard]] maui::core::thickness peek_area_insets() const
        {
            return peek_area_insets_.get();
        }
        void set_peek_area_insets(maui::core::thickness value)
        {
            peek_area_insets_.set(value);
        }

        // ---- IsBounceEnabled (default true) ----
        [[nodiscard]] bool is_bounce_enabled() const
        {
            return is_bounce_enabled_.get();
        }
        void set_is_bounce_enabled(bool value)
        {
            is_bounce_enabled_.set(value);
        }

        // ---- IsSwipeEnabled (default true) ----
        [[nodiscard]] bool is_swipe_enabled() const
        {
            return is_swipe_enabled_.get();
        }
        void set_is_swipe_enabled(bool value)
        {
            is_swipe_enabled_.set(value);
        }

        // ---- IsScrollAnimated (default true) ----
        [[nodiscard]] bool is_scroll_animated() const
        {
            return is_scroll_animated_.get();
        }
        void set_is_scroll_animated(bool value)
        {
            is_scroll_animated_.set(value);
        }

        // ---- CurrentItem (TwoWay) ----
        [[nodiscard]] const boxed_item& current_item() const
        {
            return current_item_.get();
        }
        void set_current_item(boxed_item value)
        {
            current_item_.set(std::move(value));
        }

        // ---- Position (TwoWay) ----
        [[nodiscard]] int position() const
        {
            return position_.get();
        }
        void set_position(int value)
        {
            position_.set(value);
        }

        // CarouselView.IsScrolling — renderer state (EditorBrowsable.Never).
        [[nodiscard]] bool is_scrolling() const
        {
            return is_scrolling_;
        }
        void set_is_scrolling(bool value)
        {
            is_scrolling_ = value;
        }

        // CarouselView.IsDragging — renderer-set read-only state. set_is_dragging mirrors SetIsDragging.
        [[nodiscard]] bool is_dragging() const
        {
            return is_dragging_;
        }
        void set_is_dragging(bool value)
        {
            is_dragging_ = value;
        }

        // C# AnimatePositionChanges / AnimateCurrentItemChanges (renderer reads; track IsScrollAnimated).
        [[nodiscard]] bool animate_position_changes() const
        {
            return is_scroll_animated();
        }
        [[nodiscard]] bool animate_current_item_changes() const
        {
            return is_scroll_animated();
        }

        // ---- developer-facing events + the command convention (header note) ----
        maui::core::event<const position_changed_event_args&> position_changed;
        maui::core::event<const current_item_changed_event_args&> current_item_changed;
        maui::core::move_only_function<void()> position_changed_command;
        maui::core::move_only_function<void()> current_item_changed_command;

    protected:
        // CarouselView.OnPositionChanged / OnCurrentItemChanged hooks (empty defaults).
        virtual void on_position_changed(const position_changed_event_args& args)
        {
            (void)args;
        }
        virtual void on_current_item_changed(const current_item_changed_event_args& args)
        {
            (void)args;
        }

    private:
        // PositionPropertyChanged: command → event → hook (the C# order).
        void raise_position_changed(int old_value, int new_value);
        // CurrentItemPropertyChanged: command → event → hook.
        void raise_current_item_changed(const boxed_item& old_value, const boxed_item& new_value);

        bool is_scrolling_ = false;
        bool is_dragging_ = false;

        maui::core::property<bool> loop_{*this, loop_property()};
        maui::core::property<maui::core::thickness> peek_area_insets_{*this, peek_area_insets_property()};
        maui::core::property<bool> is_bounce_enabled_{*this, is_bounce_enabled_property()};
        maui::core::property<bool> is_swipe_enabled_{*this, is_swipe_enabled_property()};
        maui::core::property<bool> is_scroll_animated_{*this, is_scroll_animated_property()};
        maui::core::property<boxed_item> current_item_{*this, current_item_property()};
        maui::core::property<int> position_{*this, position_property()};
    };
} // namespace maui::controls
