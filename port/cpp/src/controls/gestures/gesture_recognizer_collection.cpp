// maui::controls::gesture_recognizer_collection — the View.GestureRecognizers collection behavior:
// validation (View.ValidateGesture), parenting (the CollectionChanged handler's item.Parent writes),
// and the changed notification the gesture platform manager re-syncs on. See the header.

#include "maui/controls/gestures/gesture_recognizer_collection.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"

namespace maui::controls
{
    void gesture_recognizer_collection::add(std::shared_ptr<gesture_recognizer> recognizer)
    {
        if (!recognizer)
        {
            return; // ValidateGesture's null guard (C# silently skips a null item)
        }
        // View.ValidateGesture: "Only one PinchGestureRecognizer per view is allowed" —
        // InvalidOperationException in C#, std::runtime_error here (the port's stand-in).
        if (dynamic_cast<pinch_gesture_recognizer*>(recognizer.get()) != nullptr)
        {
            const bool already_has_pinch =
                std::ranges::any_of(items_, [](const std::shared_ptr<gesture_recognizer>& item) {
                    return dynamic_cast<pinch_gesture_recognizer*>(item.get()) != nullptr;
                });
            if (already_has_pinch)
            {
                throw std::runtime_error("Only one pinch_gesture_recognizer per view is allowed");
            }
        }

        gesture_recognizer& added = *recognizer;
        items_.push_back(std::move(recognizer));
        if (hooks_.attach)
        {
            hooks_.attach(added); // item.Parent = view (+ BindingContext/Window inheritance)
        }
        if (hooks_.changed)
        {
            hooks_.changed(); // CollectionChanged → LoadRecognizers
        }
    }

    bool gesture_recognizer_collection::remove(const std::shared_ptr<gesture_recognizer>& recognizer)
    {
        const auto it = std::ranges::find(items_, recognizer);
        if (it == items_.end())
        {
            return false;
        }
        std::shared_ptr<gesture_recognizer> const removed = std::move(*it); // keep alive through the hooks
        items_.erase(it);
        if (hooks_.detach)
        {
            hooks_.detach(*removed); // item.Parent = null
        }
        if (hooks_.changed)
        {
            hooks_.changed();
        }
        return true;
    }

    void gesture_recognizer_collection::clear()
    {
        if (items_.empty())
        {
            return;
        }
        // C#'s ClearItems override raises one Remove for the whole list, so every item is unparented.
        const std::vector<std::shared_ptr<gesture_recognizer>> removed = std::move(items_);
        items_.clear();
        if (hooks_.detach)
        {
            for (const auto& item : removed)
            {
                hooks_.detach(*item);
            }
        }
        if (hooks_.changed)
        {
            hooks_.changed();
        }
    }

    bool gesture_recognizer_collection::contains(const gesture_recognizer& recognizer) const
    {
        return std::ranges::any_of(items_, [&recognizer](const std::shared_ptr<gesture_recognizer>& item) {
            return item.get() == &recognizer;
        });
    }
} // namespace maui::controls
