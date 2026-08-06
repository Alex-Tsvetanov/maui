// Headless tests for the G3 effects unit — effect / null_effect / routing_effect / platform_effect, the
// explicit effect registry, and the element-side Effects collection + attach/detach lifecycle. Ported
// from src/Controls/tests/Core.UnitTests/EffectTests.cs, extended for the registry + routing_effect +
// platform_effect property-changed seam the C# tests cover implicitly.
//
// §8 ownership: the element (publisher) is declared BEFORE the effects (subscribers) it owns — the
// element's effect_collection holds the shared_ptr<effect>, so the effects never outlive it.
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/effect.hpp"
#include "maui/controls/effect_collection.hpp"
#include "maui/controls/effect_registry.hpp"
#include "maui/controls/i_effect_control_provider.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_effect.hpp"
#include "maui/controls/routing_effect.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::effect;
    using maui::controls::i_effect_control_provider;
    using maui::controls::label;
    using maui::controls::null_effect;
    using maui::controls::register_effect;
    using maui::controls::resolve_effect;
    using maui::controls::routing_effect;

    // The EffectTests.CustomEffect: records OnAttached / OnDetached and whether the provider registered it.
    class custom_effect : public effect
    {
    public:
        bool on_attached_called = false;
        bool on_detached_called = false;
        bool registered = false;
        int property_changed_count = 0;

    protected:
        void on_attached() override
        {
            on_attached_called = true;
        }
        void on_detached() override
        {
            on_detached_called = true;
        }

    public:
        void send_on_element_property_changed(std::string_view name) override
        {
            (void)name;
            ++property_changed_count;
        }
    };

    // A platform_effect that records OnElementPropertyChanged forwarding (the platform half's seam).
    class recording_platform_effect : public maui::controls::platform_effect<void, void>
    {
    public:
        int element_property_changed_count = 0;

    protected:
        void on_attached() override
        {
        }
        void on_detached() override
        {
        }
        void on_element_property_changed(std::string_view name) override
        {
            (void)name;
            ++element_property_changed_count;
        }
    };

    // The EffectTests.EffectControlProvider stub: flags a custom_effect as Registered.
    class recording_provider : public i_effect_control_provider
    {
    public:
        void register_effect(effect& target) override
        {
            ++register_count;
            if (auto* custom = dynamic_cast<custom_effect*>(&target); custom != nullptr)
            {
                custom->registered = true;
            }
        }
        int register_count = 0;
    };

    // ---- Effect.Resolve (EffectTests.ResolveSetsId / UnknownIdReturnsNullEffect) -------------------------

    TEST(effect_resolve, sets_resolve_id)
    {
        const std::string id = "Unknown";
        const std::shared_ptr<effect> resolved = effect::resolve(id);
        EXPECT_EQ(resolved->resolve_id(), id);
    }

    TEST(effect_resolve, unknown_id_returns_null_effect)
    {
        const std::shared_ptr<effect> resolved = effect::resolve("Foo");
        EXPECT_NE(dynamic_cast<null_effect*>(resolved.get()), nullptr);
    }

    // ---- The explicit registry: register -> resolve -> a registered effect, unknown -> nullptr ----------

    TEST(effect_registry, registered_id_resolves_to_the_registered_effect)
    {
        register_effect("MauiTests.Registered",
                        [] { return std::shared_ptr<effect>(std::make_shared<custom_effect>()); });

        const std::shared_ptr<effect> resolved = effect::resolve("MauiTests.Registered");
        EXPECT_NE(dynamic_cast<custom_effect*>(resolved.get()), nullptr);
        EXPECT_EQ(resolved->resolve_id(), "MauiTests.Registered"); // resolve always sets the id
    }

    TEST(effect_registry, resolve_effect_returns_null_for_unknown_id)
    {
        EXPECT_EQ(resolve_effect("MauiTests.NeverRegistered"), nullptr);
        EXPECT_FALSE(maui::controls::is_effect_registered("MauiTests.NeverRegistered"));
    }

    TEST(effect_registry, registration_is_idempotent_last_wins)
    {
        register_effect("MauiTests.Replaced", [] { return std::shared_ptr<effect>(std::make_shared<null_effect>()); });
        register_effect("MauiTests.Replaced",
                        [] { return std::shared_ptr<effect>(std::make_shared<custom_effect>()); });
        const std::shared_ptr<effect> resolved = resolve_effect("MauiTests.Replaced");
        EXPECT_NE(dynamic_cast<custom_effect*>(resolved.get()), nullptr);
    }

    // ---- send_attached / send_detached flag behavior (EffectTests.SendAttached/DetachedSetsFlag) --------

    TEST(effect_lifecycle, send_attached_sets_flag)
    {
        const std::shared_ptr<effect> resolved = effect::resolve("Foo");
        resolved->send_attached();
        EXPECT_TRUE(resolved->is_attached());
    }

    TEST(effect_lifecycle, send_detached_unsets_flag)
    {
        const std::shared_ptr<effect> resolved = effect::resolve("Foo");
        resolved->send_attached();
        resolved->send_detached();
        EXPECT_FALSE(resolved->is_attached());
    }

    TEST(effect_lifecycle, send_attached_is_idempotent)
    {
        auto custom = std::make_shared<custom_effect>();
        custom->send_attached();
        custom->on_attached_called = false; // reset to detect a second on_attached
        custom->send_attached();            // already attached -> no-op
        EXPECT_FALSE(custom->on_attached_called);
        EXPECT_TRUE(custom->is_attached());
    }

    // ---- The element lifecycle (EffectTests.EffectLifecyclePreProvider / PostProvider) -------------------

    // NB: the element's effect_collection holds an OWNING shared_ptr<effect>; removing/clearing it would
    // drop the only owner and free the effect. The tests retain their own shared_ptr (declared AFTER the
    // element, §8: publisher before subscriber) so the effect outlives the assertions on its detach flags.

    TEST(effect_element_lifecycle, attaches_when_provider_set_after_add)
    {
        label element; // publisher first (§8)
        recording_provider provider;
        auto added = std::make_shared<custom_effect>(); // retained so it survives remove

        element.effects().add(added);

        // No provider yet: not attached.
        EXPECT_FALSE(added->is_attached());
        EXPECT_FALSE(added->on_attached_called);

        element.set_effect_control_provider(&provider);

        EXPECT_TRUE(added->is_attached());
        EXPECT_TRUE(added->on_attached_called);
        EXPECT_TRUE(added->registered);
        EXPECT_FALSE(added->on_detached_called);
        EXPECT_EQ(added->attached_element(), &element);

        element.effects().remove(added.get());
        EXPECT_TRUE(added->on_detached_called);
        EXPECT_FALSE(added->is_attached());
    }

    TEST(effect_element_lifecycle, attaches_when_added_after_provider_set)
    {
        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);
        auto added = std::make_shared<custom_effect>();

        element.effects().add(added);

        EXPECT_TRUE(added->is_attached());
        EXPECT_TRUE(added->on_attached_called);
        EXPECT_TRUE(added->registered);
        EXPECT_FALSE(added->on_detached_called);

        element.effects().remove(added.get());
        EXPECT_TRUE(added->on_detached_called);
    }

    TEST(effect_element_lifecycle, clear_detaches_each_effect)
    {
        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);
        auto added = std::make_shared<custom_effect>();

        element.effects().add(added);

        element.effects().clear();
        EXPECT_TRUE(added->on_detached_called);
        EXPECT_EQ(element.effects().count(), 0U);
    }

    TEST(effect_element_lifecycle, changing_provider_redetaches_then_reattaches)
    {
        label element;
        recording_provider first;
        recording_provider second;
        auto added = std::make_shared<custom_effect>();

        element.set_effect_control_provider(&first);
        element.effects().add(added);
        ASSERT_TRUE(added->is_attached());
        added->on_attached_called = false;
        added->on_detached_called = false;

        element.set_effect_control_provider(&second);
        EXPECT_TRUE(added->on_detached_called); // detached from first
        EXPECT_TRUE(added->on_attached_called); // re-attached to second
        EXPECT_TRUE(added->is_attached());
        EXPECT_EQ(second.register_count, 1);
    }

    TEST(effect_element_lifecycle, effect_is_attached_matches_by_resolve_id)
    {
        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);

        const std::shared_ptr<effect> resolved = effect::resolve("MauiTests.Some.Effect");
        element.effects().add(resolved);

        EXPECT_TRUE(element.effect_is_attached("MauiTests.Some.Effect"));
        EXPECT_FALSE(element.effect_is_attached("MauiTests.Other.Effect"));
    }

    // Adding the same effect twice is refused (a re-add would double-attach — AttachEffect throws in C# on
    // an already-attached effect; the collection guards by ignoring the duplicate).
    TEST(effect_element_lifecycle, re_adding_the_same_effect_is_ignored)
    {
        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);

        const std::shared_ptr<effect> custom = std::make_shared<custom_effect>();
        element.effects().add(custom);
        element.effects().add(custom); // duplicate -> ignored
        EXPECT_EQ(element.effects().count(), 1U);
    }

    // ---- routing_effect resolving to a registered effect (RoutingEffect delegates its lifecycle) --------

    // A routing_effect subclass (RoutingEffect's ctor is protected — only subclasses construct it with an id).
    class my_routing_effect : public routing_effect
    {
    public:
        explicit my_routing_effect(std::string_view id) : routing_effect(id)
        {
        }
    };

    TEST(routing_effect_tests, resolves_inner_to_a_registered_effect)
    {
        register_effect("MauiTests.Routed", [] { return std::shared_ptr<effect>(std::make_shared<custom_effect>()); });

        const my_routing_effect routing("MauiTests.Routed");
        ASSERT_NE(routing.inner(), nullptr);
        EXPECT_NE(dynamic_cast<custom_effect*>(routing.inner()), nullptr);
        EXPECT_EQ(routing.inner()->resolve_id(), "MauiTests.Routed");
    }

    TEST(routing_effect_tests, unknown_id_resolves_inner_to_null_effect)
    {
        const my_routing_effect routing("MauiTests.NoSuchRoute");
        ASSERT_NE(routing.inner(), nullptr);
        EXPECT_NE(dynamic_cast<null_effect*>(routing.inner()), nullptr);
    }

    TEST(routing_effect_tests, lifecycle_delegates_to_the_inner_effect)
    {
        register_effect("MauiTests.RoutedLifecycle",
                        [] { return std::shared_ptr<effect>(std::make_shared<custom_effect>()); });

        my_routing_effect routing("MauiTests.RoutedLifecycle");
        auto* inner = dynamic_cast<custom_effect*>(routing.inner());
        ASSERT_NE(inner, nullptr);

        routing.send_attached();
        EXPECT_TRUE(inner->on_attached_called);
        EXPECT_TRUE(inner->is_attached());

        routing.send_detached();
        EXPECT_TRUE(inner->on_detached_called);
        EXPECT_FALSE(inner->is_attached());
    }

    // When the element attaches a routing_effect, it registers the INNER effect with the provider (not the
    // wrapper) — Element.AttachEffect picks re.Inner.
    TEST(routing_effect_tests, element_registers_the_inner_effect_with_the_provider)
    {
        register_effect("MauiTests.RoutedAttach",
                        [] { return std::shared_ptr<effect>(std::make_shared<custom_effect>()); });

        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);

        auto routing = std::make_shared<my_routing_effect>("MauiTests.RoutedAttach");
        auto* inner = dynamic_cast<custom_effect*>(routing->inner());
        ASSERT_NE(inner, nullptr);
        element.effects().add(std::move(routing));

        EXPECT_TRUE(inner->registered);    // the inner effect was the one registered
        EXPECT_TRUE(inner->is_attached()); // and attached
        EXPECT_EQ(inner->attached_element(), &element);
    }

    // ---- platform_effect: SendOnElementPropertyChanged forwards only while attached ---------------------

    TEST(platform_effect_tests, forwards_property_changed_only_while_attached)
    {
        recording_platform_effect platform;

        // Not attached yet: the property-changed seam is suppressed (PlatformEffect<,> guards on IsAttached).
        platform.send_on_element_property_changed("text");
        EXPECT_EQ(platform.element_property_changed_count, 0);

        platform.send_attached();
        platform.send_on_element_property_changed("text");
        EXPECT_EQ(platform.element_property_changed_count, 1);

        // After detach, the container/control are cleared and the seam suppresses again.
        platform.send_detached();
        platform.send_on_element_property_changed("text");
        EXPECT_EQ(platform.element_property_changed_count, 1);
        EXPECT_EQ(platform.native_container(), nullptr);
        EXPECT_EQ(platform.native_control(), nullptr);
    }

    // The element fans SendOnElementPropertyChanged out to its effects on a property change
    // (Element.OnPropertyChanged). A label.text change reaches the attached effect.
    TEST(effect_element_lifecycle, property_change_fans_out_to_attached_effects)
    {
        label element;
        recording_provider provider;
        element.set_effect_control_provider(&provider);

        auto custom = std::make_shared<custom_effect>();
        custom_effect* const added = custom.get();
        element.effects().add(std::move(custom));
        added->property_changed_count = 0; // ignore any changes during attach

        element.set_text("hello"); // a property change on the element
        EXPECT_GT(added->property_changed_count, 0);
    }

    // ---- the notification is re-entrancy-safe against its own owner being destroyed ----------------
    //
    // A change notification runs USER CODE in the middle of the mutation, and the stack then unwinds
    // through frames that all still touch the object: element's effects fan-out (element_effects.cpp:
    // 117-118), view's handler / visual-state / z-order push, and property<T>::set's own trailing
    // property_changed callback and `changed` raise. If a handler drops the last owning reference,
    // every one of those frames runs on freed storage.
    //
    // C# gets this for free — a managed `this` on the stack is a GC root for the whole method body, so
    // BindableObject.cs:637-644 and Element.cs:709-724 can touch `this` after their raises. The port
    // has no such root, so property<T>::set/clear pin the owner for the duration (property.hpp). These
    // two tests are the ASan witnesses for that pin; they are meaningless without -fsanitize=address.
    //
    // Not hypothetical: the binding engine subscribes to exactly these events (binding_expression.cpp,
    // bindings/binding_base.cpp), and maui::ui::view_ref owns its view by shared_ptr — so a two-way
    // binding whose view-model setter releases the view is this test.
    //
    // *** Use `shared_ptr<label>(new label)`, NEVER make_shared. *** make_shared co-allocates the
    // object with its control block, so the weak_ptr used here to prove destruction keeps the storage
    // mapped: the destructor runs but operator delete never does, ASan has nothing to poison, and the
    // test passes green against a fully live use-after-free. A separate control block is load-bearing.
    //
    // character_spacing is deliberately chosen over text: label::set_text reads formatted_text_ AFTER
    // text_.set() returns, which is a separate hazard in the CALLER and not what property<T> can fix.
    // set_character_spacing is a bare forward, so these tests isolate the notification chain itself.
    TEST(element_notification_lifetime, changed_handler_may_destroy_the_owner)
    {
        std::shared_ptr<label> owner(new label);
        std::weak_ptr<label> const observer = owner;
        owner->property_changed.connect([&owner](std::string_view) { owner.reset(); });

        label& raw = *owner;
        raw.set_character_spacing(4.0); // unwinds through every frame listed above

        EXPECT_TRUE(observer.expired()) << "test is void unless the handler really freed the label";
    }

    // The changing path reaches freed storage even harder: property<T>::set's `values_.set(...)` is a
    // WRITE into the property's own member after on_property_changing has raised.
    TEST(element_notification_lifetime, changing_handler_may_destroy_the_owner)
    {
        std::shared_ptr<label> owner(new label);
        std::weak_ptr<label> const observer = owner;
        owner->property_changing.connect([&owner](std::string_view) { owner.reset(); });

        label& raw = *owner;
        raw.set_character_spacing(4.0);

        EXPECT_TRUE(observer.expired()) << "test is void unless the handler really freed the label";
    }
} // namespace
