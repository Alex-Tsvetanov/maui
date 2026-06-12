// Tests for maui::controls::behavior / typed_behavior<T> / behavior_collection (W1-15). Ported from
// BehaviorTest.cs: AttachAndDetach, AttachToTypeCompatibleWithTargetType (C# throws
// InvalidOperationException — the no-exceptions port REFUSES: attach_to returns false),
// BehaviorsInCollectionAreAttachedWhenCollectionIsAttached, BehaviorsAddedToAttachedCollectionAreAttached,
// and TestBehaviorsAttachedDP (view.behaviors()).
#include "maui/controls/behavior.hpp"

#include <memory>

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/bindable_object.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::behavior_collection;
    using maui::controls::button;
    using maui::controls::label;
    using maui::controls::typed_behavior;
    using maui::core::bindable_object;

    // MockBehavior<T>: records the attach/detach lifecycle + the associated object. The C# mock overrides
    // the UNTYPED OnAttachedTo(BindableObject) overload and calls base — mirrored here; the
    // using-declarations re-expose the typed overloads C++ name hiding would otherwise drop.
    template <class T> struct mock_behavior : typed_behavior<T>
    {
        using typed_behavior<T>::on_attached_to;
        using typed_behavior<T>::on_detaching_from;

        bool attached = false;
        bool detached = false;
        bindable_object* associated_object = nullptr;

    protected:
        // Same visibility as the base (the hooks are protected — only the attach machinery calls them).
        void on_attached_to(bindable_object& bindable) override
        {
            typed_behavior<T>::on_attached_to(bindable);
            attached = true;
            associated_object = &bindable;
        }
        void on_detaching_from(bindable_object& bindable) override
        {
            detached = true;
            typed_behavior<T>::on_detaching_from(bindable);
            associated_object = nullptr;
        }
    };

    TEST(behavior, attach_and_detach)
    {
        mock_behavior<button> behavior;
        button bindable;

        EXPECT_FALSE(behavior.attached);
        EXPECT_FALSE(behavior.detached);
        EXPECT_EQ(behavior.associated_object, nullptr);

        EXPECT_TRUE(behavior.attach_to(bindable));
        EXPECT_TRUE(behavior.attached);
        EXPECT_FALSE(behavior.detached);
        EXPECT_EQ(behavior.associated_object, &bindable);

        behavior.detach_from(bindable);
        EXPECT_TRUE(behavior.attached);
        EXPECT_TRUE(behavior.detached);
        EXPECT_EQ(behavior.associated_object, nullptr);
    }

    TEST(behavior, attach_to_an_incompatible_target_type_is_refused)
    {
        // AttachToTypeCompatibleWithTargetType: a Behavior<button> cannot attach to a label. C# throws
        // InvalidOperationException; the port returns false and runs NO lifecycle hook.
        mock_behavior<button> behavior;
        label incompatible;

        EXPECT_FALSE(behavior.attach_to(incompatible));
        EXPECT_FALSE(behavior.attached);
        EXPECT_EQ(behavior.associated_object, nullptr);
    }

    TEST(behavior, behaviors_in_collection_are_attached_when_the_collection_is_attached)
    {
        auto behavior = std::make_shared<mock_behavior<button>>();
        behavior_collection collection;
        button bindable;
        collection.add(behavior);
        EXPECT_EQ(behavior->associated_object, nullptr); // nothing associated yet

        collection.attach_to(bindable);
        EXPECT_EQ(behavior->associated_object, &bindable);

        collection.detach_from(bindable);
        EXPECT_EQ(behavior->associated_object, nullptr);
    }

    TEST(behavior, behaviors_added_to_an_attached_collection_are_attached)
    {
        auto behavior = std::make_shared<mock_behavior<button>>();
        behavior_collection collection;
        button bindable;
        collection.attach_to(bindable);
        EXPECT_EQ(behavior->associated_object, nullptr);

        collection.add(behavior); // joining an already-attached collection attaches at once
        EXPECT_EQ(behavior->associated_object, &bindable);

        EXPECT_TRUE(collection.remove(behavior)); // leaving detaches
        EXPECT_EQ(behavior->associated_object, nullptr);
    }

    TEST(behavior, view_behaviors_collection_attaches_and_detaches)
    {
        // TestBehaviorsAttachedDP: the element's own Behaviors collection is pre-attached to it.
        auto behavior = std::make_shared<mock_behavior<button>>();
        button bindable;
        behavior_collection& collection = bindable.behaviors();
        EXPECT_EQ(behavior->associated_object, nullptr);

        collection.add(behavior);
        EXPECT_EQ(behavior->associated_object, &bindable);

        EXPECT_TRUE(collection.remove(behavior));
        EXPECT_EQ(behavior->associated_object, nullptr);
    }

    TEST(behavior, clearing_the_collection_detaches_every_behavior)
    {
        // AttachedCollection.ClearItems: every behavior detaches from every associated bindable.
        auto first = std::make_shared<mock_behavior<button>>();
        auto second = std::make_shared<mock_behavior<button>>();
        button bindable;
        bindable.behaviors().add(first);
        bindable.behaviors().add(second);
        EXPECT_EQ(first->associated_object, &bindable);
        EXPECT_EQ(second->associated_object, &bindable);

        bindable.behaviors().clear();
        EXPECT_TRUE(first->detached);
        EXPECT_TRUE(second->detached);
        EXPECT_EQ(bindable.behaviors().count(), 0U);
    }
} // namespace
