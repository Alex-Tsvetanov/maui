#pragma once
// maui::controls::gesture_recognizer_collection  <=  the ObservableCollection<IGestureRecognizer>
// behind View.GestureRecognizers (View.cs's _gestureRecognizers + its CollectionChanged handler +
// View.ValidateGesture).
//
// The mutable recognizer list a view exposes through gesture_recognizers(). Each mutation
//   1. validates the candidate (View.ValidateGesture: only ONE pinch recognizer per view — a second
//      add throws std::runtime_error, the port's stand-in for InvalidOperationException; the port
//      validates BEFORE inserting, where C# validates from the collection-changed handler after the
//      insert — same observable outcome, the throw);
//   2. parents / unparents the recognizer via the owner-supplied hooks (C#: item.Parent = this on add,
//      null on remove — view<> attaches the recognizer as a logical child so BindingContext + Window
//      inherit);
//   3. raises the changed hook (CollectionChanged), which the view routes to the gesture platform
//      manager's load_recognizers — exactly how C#'s GesturePlatformManager observes the collection.
//
// Ownership (PROFILE §8): the collection OWNS the recognizers (shared_ptr — C#'s collection holds the
// only strong reference too); the hooks borrow the owning view.

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/gestures/gesture_recognizer.hpp"

namespace maui::controls
{
    class gesture_recognizer_collection
    {
    public:
        // The owning view's callbacks: attach/detach parent the recognizer into the view's logical
        // tree; changed re-syncs the platform manager; validate is the owner's View.ValidateGesture
        // override run BEFORE insert (it throws to reject a candidate — e.g. a span allows only tap).
        // All four may be empty (tests / the common view case validates pinch internally below).
        struct hooks
        {
            std::function<void(gesture_recognizer&)> attach;
            std::function<void(gesture_recognizer&)> detach;
            std::function<void()> changed;
            std::function<void(const gesture_recognizer&)> validate;
        };

        explicit gesture_recognizer_collection(hooks owner_hooks) : hooks_(std::move(owner_hooks))
        {
        }
        // Non-copyable/non-movable: the hooks back-reference the owning view (PROFILE §8).
        gesture_recognizer_collection(const gesture_recognizer_collection&) = delete;
        gesture_recognizer_collection(gesture_recognizer_collection&&) = delete;
        gesture_recognizer_collection& operator=(const gesture_recognizer_collection&) = delete;
        gesture_recognizer_collection& operator=(gesture_recognizer_collection&&) = delete;
        ~gesture_recognizer_collection() = default;

        // Add a recognizer (a null pointer is ignored, like C#'s ValidateGesture null guard). Throws
        // std::runtime_error when adding a second pinch recognizer (View.ValidateGesture).
        void add(std::shared_ptr<gesture_recognizer> recognizer);
        // Remove a recognizer; returns true iff it was present (unparents it on the way out).
        bool remove(const std::shared_ptr<gesture_recognizer>& recognizer);
        // Remove every recognizer (the C# GestureRecognizerCollection.ClearItems override raises a
        // Remove for the whole list so each item is unparented — mirrored here).
        void clear();

        [[nodiscard]] std::size_t count() const
        {
            return items_.size();
        }
        [[nodiscard]] bool contains(const gesture_recognizer& recognizer) const;
        [[nodiscard]] const std::shared_ptr<gesture_recognizer>& at(std::size_t index) const
        {
            return items_[index];
        }
        [[nodiscard]] const std::vector<std::shared_ptr<gesture_recognizer>>& items() const
        {
            return items_;
        }

    private:
        hooks hooks_;
        std::vector<std::shared_ptr<gesture_recognizer>> items_;
    };
} // namespace maui::controls
