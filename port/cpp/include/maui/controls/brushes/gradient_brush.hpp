#pragma once
// maui::controls::gradient_brush  <=  Microsoft.Maui.Controls.GradientBrush (+ GradientStopCollection)
//
// Abstract base of the gradient brushes (linear / radial). Ported from src/Controls/src/Core/GradientBrush.cs
// and GradientStopCollection.cs: it owns a collection of gradient_stop children, propagates its
// BindingContext into them (OnBindingContextChanged → SetInheritedBindingContext), parents each stop
// (UpdateGradientStops: newStop.Parent = this), and raises invalidate_gradient_brush_requested when the
// collection or any stop changes (the InvalidateGradientBrushRequested event the VisualElement subscribes).
// IsEmpty is "no stops" (GradientStops null/empty).
//
// COLLECTION MODEL: C#'s GradientStopCollection is an ObservableCollection<GradientStop>; the port models
// it as gradient_stop_collection — a vector<shared_ptr<gradient_stop>> with Add/Insert/Count/indexing that
// notifies its owning brush on every mutation (the CollectionChanged → attach/detach + Invalidate path).
// Stops are shared_ptr so the brush owns them (the element tree, PROFILE §8) and a stop can be shared into
// the brush→paint bridge by borrow. A null entry is allowed (the C# `{ null, null }` tests) and is simply
// skipped for parenting / paint conversion.
//
// Out-of-line definitions live in gradient_brush.cpp.

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class gradient_brush; // forward (the collection back-refs its owner)

    // Microsoft.Maui.Controls.GradientStopCollection — the observable stop list. Owned BY a gradient_brush;
    // every mutation calls back into the owner so it can (un)parent stops + re-raise the invalidate event.
    class gradient_stop_collection
    {
    public:
        using container = std::vector<std::shared_ptr<gradient_stop>>;

        explicit gradient_stop_collection(gradient_brush& owner) : owner_(&owner)
        {
        }

        // ObservableCollection.Add / Insert / Count / indexer + iteration (the surface the tests + converter
        // use). Each mutating call notifies the owner (attach/detach the stop + Invalidate).
        void add(std::shared_ptr<gradient_stop> stop);
        void insert(std::size_t index, std::shared_ptr<gradient_stop> stop);
        void clear();
        [[nodiscard]] std::size_t count() const
        {
            return stops_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return stops_.empty();
        }
        [[nodiscard]] const std::shared_ptr<gradient_stop>& operator[](std::size_t index) const
        {
            return stops_[index];
        }
        [[nodiscard]] container::const_iterator begin() const
        {
            return stops_.begin();
        }
        [[nodiscard]] container::const_iterator end() const
        {
            return stops_.end();
        }
        [[nodiscard]] const container& items() const
        {
            return stops_;
        }

    private:
        gradient_brush* owner_; // non-owning back-ref (the brush owns this collection)
        container stops_;
    };

    class gradient_brush : public brush
    {
    public:
        // C# GradientBrush.InvalidateGradientBrushRequested — raised when the stops change (collection or a
        // stop property), so a hosting VisualElement re-renders its background.
        maui::core::event<> invalidate_gradient_brush_requested;

        // C# GradientBrush.GradientStops — the owned stop collection (mutable, observable).
        [[nodiscard]] gradient_stop_collection& gradient_stops()
        {
            return stops_;
        }
        [[nodiscard]] const gradient_stop_collection& gradient_stops() const
        {
            return stops_;
        }
        // Replace the whole collection (the C# `GradientStops = new GradientStopCollection { … }` setter):
        // detach the old stops, take the new ones, parent them, and invalidate.
        void set_gradient_stops(std::vector<std::shared_ptr<gradient_stop>> value);

        // C# GradientBrush.IsEmpty — GradientStops is null or empty.
        [[nodiscard]] bool is_empty() const override
        {
            return stops_.empty();
        }

    protected:
        gradient_brush();

        // Every stop is a logical child (so it inherits BindingContext + participates in the tree); the base
        // element::on_binding_context_changed propagates this brush's context to each via this visitor —
        // matching C# GradientBrush.OnBindingContextChanged's SetInheritedBindingContext loop.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        friend class gradient_stop_collection;
        // The collection's mutation hooks (the CollectionChanged add/remove branches): parent/unparent the
        // stop, (un)subscribe its property_changed, and raise the invalidate event.
        void on_stop_added(gradient_stop& stop);
        static void on_stop_removed(gradient_stop& stop);
        void raise_invalidate() const;

        gradient_stop_collection stops_{*this};
        // Per-stop property_changed subscriptions, parallel to stops_ (dropped on clear/replace). Stored as
        // shared_ptr-keyed tokens via a vector kept in lockstep with the collection's mutations.
        std::vector<maui::core::scoped_connection> stop_tokens_;
    };
} // namespace maui::controls
