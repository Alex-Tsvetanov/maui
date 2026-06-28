// Tests for maui::core::observable<T> — the one-member bindable VM property (PUBLIC_API_DESIGN.md §3-A).
//
// Proves it replaces the static-descriptor + property-member boilerplate while preserving everything the
// binding engine needs: get/set + a (old,new) changed event, name self-registration (so the stringly
// set_binding path resolves it), and the typed bind() bridge via as_property().

#include "maui/core/observable.hpp"

#include "maui/core/bindable_object.hpp"
#include "maui/core/binding.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{
    using maui::core::bindable_object;
    using maui::core::observable;

    // The one-member view-model property — no static descriptor function, no `{*this, descriptor()}`.
    class greeting_vm : public bindable_object
    {
    public:
        observable<std::string> message{*this, "message", "World"};
    };

    // Non-movable (the descriptor borrows a string_view into name_, property holds an owner back-pointer).
    static_assert(!std::is_move_constructible_v<observable<std::string>>);
    static_assert(!std::is_copy_constructible_v<observable<std::string>>);

    TEST(observable, get_set_and_changed)
    {
        greeting_vm vm;
        EXPECT_EQ(vm.message.get(), "World"); // the default

        std::string seen;
        auto conn = maui::core::connect_scoped(
            vm.message.changed(), [&seen](const std::string& /*old*/, const std::string& nv) { seen = nv; });

        vm.message.set("Hi");
        EXPECT_EQ(vm.message.get(), "Hi");
        EXPECT_EQ(seen, "Hi");
    }

    TEST(observable, self_registers_for_the_stringly_binding_path)
    {
        const greeting_vm vm;
        // The inner property registered "message" on the owner, so the name->value channel resolves it.
        EXPECT_TRUE(vm.has_property("message"));
        EXPECT_TRUE(vm.try_get_value("message").has_value());
    }

    TEST(observable, typed_bind_against_as_property)
    {
        greeting_vm src;
        greeting_vm dst;
        src.message.set("X");

        auto handle =
            maui::core::bind(dst.message.as_property(), src.message.as_property(), maui::core::binding_mode::one_way);
        EXPECT_EQ(dst.message.get(), "X"); // initial push at bind time

        src.message.set("Y");
        EXPECT_EQ(dst.message.get(), "Y"); // propagates through the binding
    }
} // namespace
