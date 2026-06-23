#pragma once
// maui::controls::swipe_view  <=  Microsoft.Maui.Controls.SwipeView
//
// A view that provides context-specific swipe interactions: a single Content plus four directional
// SwipeItems collections (Left/Right/Top/Bottom) revealed on a swipe. Ported from
// src/Controls/src/Core/SwipeView/SwipeView.cs (SwipeView : ContentView, ISwipeView):
//   - The four item collections each have a defaultValueCreator (an empty SwipeItems) and are parented
//     as logical children (so BindingContext inherits into them and their items). Replacing a collection
//     re-parents: the old one is detached, the new one attached (BindingContextTransfersToNewSetOfSwipeItems).
//   - IsOpen is a settable state the platform writes back; setting it (when it changes) notifies the handler.
//   - Open(item, animated) / Close(animated) raise the OpenRequested / CloseRequested events, then route a
//     RequestOpen / RequestClose command to the handler (the C# Open/Close pair).
//   - SwipeStarted / SwipeChanging / SwipeEnded are the inbound notifications the platform (the swipe
//     state machine) raises; the control forwards them to its swipe_started / swipe_changing / swipe_ended
//     events (the SwipeStartedEventArgs / -Changing / -Ended pair, collapsed to the request records).
//   - Threshold + SwipeTransitionMode (default Reveal) round out the surface.
//
// API shape: bare-noun i_swipe_view getters + method accessors over private property<T> engines, the
// port convention. Content is a non-owning child pointer (content_page recipe); the four collections are
// OWNED by the control (swipe_view co-owns them like C#'s defaultValueCreator-minted SwipeItems — the
// developer rarely owns them, so the control holds them by unique_ptr and hands out the i_swipe_items face).
//
// DEVIATIONS (documented): C#'s ControlTemplate hosting (the SwipeView is a ContentView and supports a
// control template) is NOT ported — the port's templated_view base is incompatible with the
// i_swipe_view : i_content_view contract diamond, so swipe_view follows the direct view<i_*> content-host
// recipe (border / scroll_view / content_page) instead, with the same content BindingContext propagation.
// The parent-scroll auto-close (subscribing to an ancestor ScrollView/ListView/CollectionView's Scrolled
// to close on scroll) is also out of scope — those legacy item-view ancestors don't exist in the port.

#include <functional>
#include <memory>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/swipe_items.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_swipe_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/property.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/core/swipe_transition_mode.hpp"
#include "maui/core/swipe_view_requests.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class swipe_view : public view<maui::core::i_swipe_view>
    {
    public:
        swipe_view();
        ~swipe_view() override;
        swipe_view(const swipe_view&) = delete;
        swipe_view(swipe_view&&) = delete;
        swipe_view& operator=(const swipe_view&) = delete;
        swipe_view& operator=(swipe_view&&) = delete;

        // Shared bindable-property descriptors (SwipeView.ThresholdProperty + the page Padding).
        static const maui::core::bindable_property<double>& threshold_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // ---- events ----
        // C# SwipeView.SwipeStarted / SwipeChanging / SwipeEnded.
        maui::core::event<maui::core::swipe_view_swipe_started> swipe_started;
        maui::core::event<maui::core::swipe_view_swipe_changing> swipe_changing;
        maui::core::event<maui::core::swipe_view_swipe_ended> swipe_ended;
        // C# SwipeView.OpenRequested / CloseRequested (EditorBrowsable-Never, but observable).
        maui::core::event<maui::core::swipe_view_open_request> open_requested;
        maui::core::event<maui::core::swipe_view_close_request> close_requested;

        // ---- Content (non-owning child; the content_page recipe) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }
        void set_content(maui::core::i_view& value)
        {
            set_content(&value);
        }
        void set_content(maui::core::i_view* value);

        // ---- i_padding ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }

        // ---- Threshold ----
        [[nodiscard]] double threshold() const override
        {
            return threshold_.get();
        }
        void set_threshold(double value)
        {
            threshold_.set(value);
        }

        // ---- the four directional item collections (owned; the i_swipe_items face is handed out) ----
        [[nodiscard]] swipe_items& left_items_collection()
        {
            return *left_items_;
        }
        [[nodiscard]] swipe_items& right_items_collection()
        {
            return *right_items_;
        }
        [[nodiscard]] swipe_items& top_items_collection()
        {
            return *top_items_;
        }
        [[nodiscard]] swipe_items& bottom_items_collection()
        {
            return *bottom_items_;
        }
        // Replace a whole collection (C# SwipeView.LeftItems setter): detach the old, attach the new,
        // re-subscribe the change forwarding, and notify the handler. Takes ownership of `value`.
        void set_left_items(std::unique_ptr<swipe_items> value);
        void set_right_items(std::unique_ptr<swipe_items> value);
        void set_top_items(std::unique_ptr<swipe_items> value);
        void set_bottom_items(std::unique_ptr<swipe_items> value);

        // ---- i_swipe_view item getters (the handler-facing face) ----
        [[nodiscard]] maui::core::i_swipe_items* left_items() const override
        {
            return left_items_.get();
        }
        [[nodiscard]] maui::core::i_swipe_items* right_items() const override
        {
            return right_items_.get();
        }
        [[nodiscard]] maui::core::i_swipe_items* top_items() const override
        {
            return top_items_.get();
        }
        [[nodiscard]] maui::core::i_swipe_items* bottom_items() const override
        {
            return bottom_items_.get();
        }

        // ---- IsOpen (the platform writes it back; a change notifies the handler) ----
        [[nodiscard]] bool is_open() const override
        {
            return is_open_;
        }
        void set_is_open(bool value) override;

        // ---- SwipeTransitionMode (default Reveal; a platform config, read-only at this layer) ----
        [[nodiscard]] maui::core::swipe_transition_mode transition_mode() const override
        {
            return transition_mode_;
        }
        void set_transition_mode(maui::core::swipe_transition_mode value)
        {
            transition_mode_ = value;
        }

        // ---- Open / Close (the developer API: raise the request event, then route the command) ----
        void open(maui::core::open_swipe_item item, bool animated = true);
        void close(bool animated = true);

        // ---- i_swipe_view inbound notifications (the platform raises these; forward to the events) ----
        void notify_swipe_started(const maui::core::swipe_view_swipe_started& args) override
        {
            last_swipe_direction_ = args.direction;
            swipe_started.raise(args);
        }
        void notify_swipe_changing(const maui::core::swipe_view_swipe_changing& args) override
        {
            swipe_changing.raise(args);
        }
        void notify_swipe_ended(const maui::core::swipe_view_swipe_ended& args) override
        {
            last_swipe_direction_ = args.direction;
            swipe_ended.raise(args);
        }

        // ---- i_swipe_view request commands (C# ISwipeView.RequestOpen / RequestClose) ----
        void request_open(const maui::core::swipe_view_open_request& request) override;
        void request_close(const maui::core::swipe_view_close_request& request) override;

        // ---- layout pass: MeasureContent / ArrangeContent within the padding (content_page recipe) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        // Content + the four collections are this view's logical children (BindingContext inherits down).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

        // Generic mount (app_host): re-fire "set_content" so the now-attached handler hosts the swiped
        // content's native view (the construction-order replay of set_content's command).
        void mount_into_handler() override
        {
            if (const auto& element_handler = handler())
            {
                element_handler->invoke("set_content");
            }
        }

    private:
        // Replace one of the four collections (the shared body for the four setters): reset the old
        // collection's change subscription (disconnecting while the old collection is still alive — §8),
        // detach the old collection, move in + attach the new, re-subscribe, and notify the handler.
        void replace_items(std::unique_ptr<swipe_items>& slot, maui::core::scoped_connection& sub,
                           std::unique_ptr<swipe_items> value, const char* update_name);
        // Subscribe to a collection's `changed` to re-push the items to the handler (C# OnSwipeItemsChanged
        // wires CollectionChanged/PropertyChanged → Handler.UpdateValue(nameof(...))).
        maui::core::scoped_connection subscribe_items(swipe_items& items, const char* update_name);

        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        // The four collections are OWNED (C#'s defaultValueCreator mints them; the developer rarely owns
        // them). Created in the ctor, parented as logical children. Declared BEFORE the subscriptions so
        // the scoped_connections (subscribers) destruct first — publishers outlive subscribers (§8).
        std::unique_ptr<swipe_items> left_items_;
        std::unique_ptr<swipe_items> right_items_;
        std::unique_ptr<swipe_items> top_items_;
        std::unique_ptr<swipe_items> bottom_items_;
        // The per-collection change subscriptions (RAII; disconnect before the collections die — §8).
        maui::core::scoped_connection left_items_sub_;
        maui::core::scoped_connection right_items_sub_;
        maui::core::scoped_connection top_items_sub_;
        maui::core::scoped_connection bottom_items_sub_;

        bool is_open_ = false;
        maui::core::swipe_transition_mode transition_mode_ = maui::core::swipe_transition_mode::reveal;
        maui::core::swipe_direction last_swipe_direction_ = maui::core::swipe_direction::none;

        maui::core::property<double> threshold_{*this, threshold_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
    };
} // namespace maui::controls
