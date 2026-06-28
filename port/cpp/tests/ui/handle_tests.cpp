// Handle-semantics tests for maui::ui::view_ref<T> / weak_ref<T> (PUBLIC_API_DESIGN.md §2.5).
//
// Proves the riskiest claims of the move-only-handle decision: the owner is move-only (so capturing it
// into its own handler is a COMPILE ERROR), the observer is copyable and dangling-safe, .share() mints a
// visible second owner, and a weak self-capture does not keep the control alive (the no-leak proof).

#include "maui/ui/builder.hpp"
#include "maui/ui/view_ref.hpp"

#include "maui/controls/label.hpp"

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

namespace
{
    namespace ui = maui::ui;
    using maui::controls::label;

    // The ownership contract, in the type system: owner is move-only (so `[handle]{...}` self-capture in an
    // on_click is ill-formed — the cycle footgun won't compile), observer is copyable.
    static_assert(!std::is_copy_constructible_v<ui::view_ref<label>>,
                  "view_ref must be move-only so capturing it into its own handler won't compile");
    static_assert(!std::is_copy_assignable_v<ui::view_ref<label>>);
    static_assert(std::is_nothrow_move_constructible_v<ui::view_ref<label>>);
    static_assert(std::is_copy_constructible_v<ui::weak_ref<label>>);

    TEST(ui_handle, weak_lock_roundtrip_and_dangling_safe)
    {
        ui::weak_ref<label> w;
        EXPECT_FALSE(w.alive());
        {
            auto owner = ui::label("hi"); // view_ref<label> owns the control
            w = owner.weak();
            EXPECT_TRUE(w.alive());

            auto locked = w.lock(); // transient owner to dot into
            ASSERT_TRUE(static_cast<bool>(locked));
            EXPECT_EQ(std::string(locked->text()), "hi"); // operator-> reaches the control
        }
        EXPECT_FALSE(w.alive());                   // owner dropped -> control destroyed -> observer expired
        EXPECT_FALSE(static_cast<bool>(w.lock())); // lock() empty, no UB (dangling-safe)
    }

    TEST(ui_handle, share_mints_a_visible_second_owner)
    {
        ui::weak_ref<label> w;
        {
            auto a = ui::label("x");
            w = a.weak();
            EXPECT_EQ(a.shared().use_count(), 1L);
            {
                auto b = a.share(); // explicit co-owner
                EXPECT_EQ(a.shared().use_count(), 2L);
            }
            EXPECT_EQ(a.shared().use_count(), 1L); // co-owner dropped
            EXPECT_TRUE(w.alive());
        }
        EXPECT_FALSE(w.alive());
    }

    TEST(ui_handle, weak_self_capture_does_not_keep_the_control_alive)
    {
        ui::weak_ref<label> w;
        {
            auto owner = ui::label("z");
            w = owner.weak();
            // The blessed self-capture: a closure holding only a weak_ref does NOT extend the lifetime.
            // (Capturing `owner` itself — [owner]{...} — would not compile: view_ref's copy ctor is deleted.)
            auto closure = [obs = owner.weak()] { return obs.alive(); };
            EXPECT_TRUE(closure());
        }
        EXPECT_FALSE(w.alive()); // destroyed despite the weak closure capture
    }
} // namespace
