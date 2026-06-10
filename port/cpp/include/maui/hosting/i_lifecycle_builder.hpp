#pragma once
// maui::hosting::i_lifecycle_builder  <=  Microsoft.Maui.LifecycleEvents.ILifecycleBuilder
//   (+ Microsoft.Maui.LifecycleEvents.LifecycleBuilderExtensions.AddEvent)
//
// The registration face ConfigureLifecycleEvents delegates receive: named multicast delegate lists.
// C# stores System.Delegate and retrieves with OfType<TDelegate>(); the C++ twin erases each delegate
// into a std::any holding its canonical std::function type — boundary-confined erasure exactly like the
// service_registry (the registry is inherently heterogeneous; PROFILE §7's carve-out), never the value
// system. Register with the typed add_event<TDelegate>(...) (TDelegate must BE a std::function
// specialization — it is the any_cast key get_event_delegates<TDelegate> retrieves by), or with the
// no-payload convenience overload, where any nullary callable converts to lifecycle_action.

#include <any>
#include <functional>
#include <string_view>
#include <utility>

namespace maui::hosting
{
    // The no-payload delegate (the C# Action shape) — the default lifecycle event delegate.
    using lifecycle_action = std::function<void()>;

    // The delegate-shape trait the typed add_event requires: only std::function specializations are
    // retrievable (the retrieval side any_casts back to the exact std::function type).
    template <class T> inline constexpr bool is_lifecycle_delegate = false;
    template <class Signature> inline constexpr bool is_lifecycle_delegate<std::function<Signature>> = true;

    class i_lifecycle_builder
    {
    public:
        virtual ~i_lifecycle_builder() = default;

        // The erased primitive: append `action` (a std::any holding a std::function) to the named list.
        virtual void add_event_delegate(std::string_view event_name, std::any action) = 0;

        // ILifecycleBuilder.AddEvent<TDelegate>(eventName, action). TDelegate is the std::function type
        // the consumer later retrieves with get_event_delegates<TDelegate>.
        template <class TDelegate>
            requires is_lifecycle_delegate<TDelegate>
        void add_event(std::string_view event_name, TDelegate action)
        {
            add_event_delegate(event_name, std::any(std::move(action)));
        }
        // LifecycleBuilderExtensions.AddEvent(eventName, Action): the no-payload convenience — lambdas
        // land here (the constrained template rejects non-std::function callables).
        void add_event(std::string_view event_name, lifecycle_action action)
        {
            add_event<lifecycle_action>(event_name, std::move(action));
        }

    protected:
        i_lifecycle_builder() = default;
        i_lifecycle_builder(const i_lifecycle_builder&) = default;
        i_lifecycle_builder(i_lifecycle_builder&&) = default;
        i_lifecycle_builder& operator=(const i_lifecycle_builder&) = default;
        i_lifecycle_builder& operator=(i_lifecycle_builder&&) = default;
    };
} // namespace maui::hosting
