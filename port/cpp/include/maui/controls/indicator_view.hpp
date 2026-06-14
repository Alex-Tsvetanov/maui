#pragma once
// maui::controls::indicator_view  <=  Microsoft.Maui.Controls.IndicatorView
//
// A view that draws dots representing the count and current position of a collection — typically
// synced to a carousel_view (CarouselView.IndicatorView binds the indicator's Position + ItemsSource
// to the carousel). Ported from src/Controls/src/Core/IndicatorView/IndicatorView.cs.
//
// Shape notes (deviations documented):
//   - C# IndicatorView is a TemplatedView (it can host a custom IndicatorTemplate via an
//     IndicatorStackLayout); the port derives view<i_indicator_view> and OMITS the template path
//     (IndicatorTemplate / IndicatorLayout / IndicatorStackLayout). The default-dot rendering — the
//     only path the native UIPageControl / NSStackView handler drives — is fully modeled (documented).
//   - ItemsSource is the erased i_item_collection seam (like items_view): setting it subscribes to the
//     live collection and pushes Count = collection.Count (the C# ResetItemsSource / OnCollectionChanged
//     → Count). The typed set_items_source<T> overloads wrap an observable_collection / vector.
//   - Position is TwoWay; set_position_from_handler writes a native dot tap back at FromHandler
//     specificity (the C# IIndicatorView.Position setter / SetterSpecificity.FromHandler).
//   - The Count propertyChanged (C# resets the IndicatorStackLayout children) has no port analog
//     without the template path; Count simply drives the handler's UpdateIndicatorCount mapper.

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/indicator_shape.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_indicator_view.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class indicator_view : public view<maui::core::i_indicator_view>
    {
    public:
        indicator_view();

        // Shared descriptors (IndicatorView.*Property).
        static const maui::core::bindable_property<maui::controls::indicator_shape>& indicators_shape_property();
        static const maui::core::bindable_property<int>& position_property();
        static const maui::core::bindable_property<int>& count_property();
        static const maui::core::bindable_property<int>& maximum_visible_property();
        static const maui::core::bindable_property<bool>& hide_single_property();
        static const maui::core::bindable_property<maui::graphics::color>& indicator_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& selected_indicator_color_property();
        static const maui::core::bindable_property<double>& indicator_size_property();
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>>& items_source_property();

        // ---- i_indicator_view (read by the handler's mapper) ----
        [[nodiscard]] int count() const override
        {
            return count_.get();
        }
        [[nodiscard]] int position() const override
        {
            return position_.get();
        }
        // IIndicatorView.Position setter (the inbound channel): a native dot tap writes back at
        // FromHandler specificity (the C# SetValue(PositionProperty, value, SetterSpecificity.FromHandler)).
        void set_position(int value) override
        {
            position_.set(value, maui::core::setter_specificity::from_handler);
        }
        [[nodiscard]] int maximum_visible() const override
        {
            return maximum_visible_.get();
        }
        [[nodiscard]] bool hide_single() const override
        {
            return hide_single_.get();
        }
        [[nodiscard]] double indicator_size() const override
        {
            return indicator_size_.get();
        }
        [[nodiscard]] maui::graphics::color indicator_color() const override
        {
            return indicator_color_.get();
        }
        [[nodiscard]] maui::graphics::color selected_indicator_color() const override
        {
            return selected_indicator_color_.get();
        }
        [[nodiscard]] maui::controls::indicator_shape indicators_shape() const override
        {
            return indicators_shape_.get();
        }

        // ---- developer-facing setters (manual specificity) ----
        void set_count(int value)
        {
            count_.set(value);
        }
        // IndicatorView.Position setter (the developer/binding face — manual specificity).
        void set_position_manual(int value)
        {
            position_.set(value);
        }
        void set_maximum_visible(int value)
        {
            maximum_visible_.set(value);
        }
        void set_hide_single(bool value)
        {
            hide_single_.set(value);
        }
        void set_indicator_size(double value)
        {
            indicator_size_.set(value);
        }
        void set_indicator_color(maui::graphics::color value)
        {
            indicator_color_.set(value);
        }
        void set_selected_indicator_color(maui::graphics::color value)
        {
            selected_indicator_color_.set(value);
        }
        void set_indicators_shape(maui::controls::indicator_shape value)
        {
            indicators_shape_.set(value);
        }

        // ---- ItemsSource (typed entries over the erased seam; drives Count) ----
        [[nodiscard]] const std::shared_ptr<i_item_collection>& items_source() const
        {
            return items_source_.get();
        }
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
        void clear_items_source()
        {
            items_source_.set(nullptr);
        }

    private:
        // ResetItemsSource: re-subscribe to the live collection + recompute Count (the C#
        // OnCollectionChanged → Count = collection.Count).
        void reset_items_source();
        void refresh_count_from_items_source();

        std::shared_ptr<i_item_collection> tracked_source_;  // PINNED so the subscription never outlives it (§8)
        maui::core::scoped_connection items_source_changed_; // after tracked_source_ (§8)

        maui::core::property<maui::controls::indicator_shape> indicators_shape_{*this, indicators_shape_property()};
        maui::core::property<int> position_{*this, position_property()};
        maui::core::property<int> count_{*this, count_property()};
        maui::core::property<int> maximum_visible_{*this, maximum_visible_property()};
        maui::core::property<bool> hide_single_{*this, hide_single_property()};
        maui::core::property<maui::graphics::color> indicator_color_{*this, indicator_color_property()};
        maui::core::property<maui::graphics::color> selected_indicator_color_{*this,
                                                                              selected_indicator_color_property()};
        maui::core::property<double> indicator_size_{*this, indicator_size_property()};
        maui::core::property<std::shared_ptr<i_item_collection>> items_source_{*this, items_source_property()};
    };
} // namespace maui::controls
