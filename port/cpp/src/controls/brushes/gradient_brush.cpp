// maui::controls::gradient_brush + gradient_stop_collection out-of-line definitions
// (header: brushes/gradient_brush.hpp).

#include "maui/controls/brushes/gradient_brush.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/brushes/gradient_stop.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    // ---- gradient_stop_collection ----

    void gradient_stop_collection::add(std::shared_ptr<gradient_stop> stop)
    {
        gradient_stop* raw = stop.get();
        stops_.push_back(std::move(stop));
        // CollectionChanged add-branch: parent + subscribe the new stop, then Invalidate.
        if (raw != nullptr)
        {
            owner_->on_stop_added(*raw);
        }
        else
        {
            owner_->raise_invalidate(); // a null entry still re-renders (keeps token parity, see owner)
        }
    }

    void gradient_stop_collection::insert(std::size_t index, std::shared_ptr<gradient_stop> stop)
    {
        gradient_stop* raw = stop.get();
        index = std::min(index, stops_.size());
        stops_.insert(stops_.begin() + static_cast<container::difference_type>(index), std::move(stop));
        if (raw != nullptr)
        {
            owner_->on_stop_added(*raw);
        }
        else
        {
            owner_->raise_invalidate();
        }
    }

    void gradient_stop_collection::clear()
    {
        for (const auto& stop : stops_)
        {
            if (stop)
            {
                gradient_brush::on_stop_removed(*stop);
            }
        }
        stops_.clear();
        // Drop the per-stop property_changed subscriptions too: otherwise a token would outlive its stop
        // (a dangling connection on the next invalidate). owner_->stop_tokens_ is accessible — gradient_brush
        // friends this collection.
        owner_->stop_tokens_.clear();
        owner_->raise_invalidate();
    }

    // ---- gradient_brush ----

    gradient_brush::gradient_brush() = default;

    void gradient_brush::set_gradient_stops(std::vector<std::shared_ptr<gradient_stop>> value)
    {
        // C# GradientStops setter → UpdateGradientStops(old, new): detach the old stops, swap in the new,
        // parent + subscribe each, then invalidate.
        for (const auto& old_stop : stops_.items())
        {
            if (old_stop)
            {
                on_stop_removed(*old_stop);
            }
        }
        stop_tokens_.clear();
        stops_ = gradient_stop_collection{*this};
        for (auto& stop : value)
        {
            stops_.add(std::move(stop)); // add() re-parents + subscribes + invalidates per element
        }
    }

    void gradient_brush::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const auto& stop : stops_.items())
        {
            if (stop)
            {
                visit(*stop);
            }
        }
    }

    void gradient_brush::on_stop_added(gradient_stop& stop)
    {
        // CollectionChanged add: AddLogicalChild(stop) (parents it + flows BindingContext down) and subscribe
        // its PropertyChanged so a later Color/Offset change re-renders. Then Invalidate.
        attach_logical_child(stop);
        stop_tokens_.push_back(
            maui::core::connect_scoped(stop.property_changed, [this](std::string_view) { raise_invalidate(); }));
        raise_invalidate();
    }

    void gradient_brush::on_stop_removed(gradient_stop& stop)
    {
        // CollectionChanged remove: unparent the stop (its parallel token is dropped by the caller clearing
        // stop_tokens_).
        element::detach_logical_child(stop);
    }

    void gradient_brush::raise_invalidate() const
    {
        // C# GradientBrush.Invalidate — fire InvalidateGradientBrushRequested only (the signal the owning
        // VisualElement subscribes to re-render its background); C#'s Invalidate raises nothing else.
        invalidate_gradient_brush_requested.raise();
    }
} // namespace maui::controls
