// Tests for maui::animations::animation + lerping_animation. The C# oracle has no dedicated
// Animation unit suite (its behavior is covered through the Controls animation tests); these cases
// capture the source behavior of src/Core/src/Animations/Animation.cs + LerpingAnimation.cs per the
// porting loop ("when there's no test, write one that captures the source's behavior").
#include "maui/animations/animation.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <vector>

#include "maui/animations/easing.hpp"
#include "maui/animations/lerping_animation.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::animations::animation;
    using maui::animations::easing;
    using maui::animations::lerping_animation;

    TEST(animation, defaults_match_the_oracle)
    {
        const auto subject = std::make_shared<animation>();
        EXPECT_EQ(subject->start_delay(), 0.0);
        EXPECT_EQ(subject->duration(), 1.0);
        EXPECT_EQ(subject->current_time(), 0.0);
        EXPECT_EQ(subject->progress(), 0.0);
        EXPECT_FALSE(subject->has_finished());
        EXPECT_FALSE(subject->repeats());
        EXPECT_FALSE(subject->is_paused());
        EXPECT_FALSE(subject->is_disposed());
        // Easing.Default = CubicInOut.
        subject->update(0.25);
        EXPECT_DOUBLE_EQ(subject->progress(), easing::cubic_in_out().ease(0.25));
    }

    TEST(animation, update_eases_progress_steps_and_finishes_at_one)
    {
        std::vector<double> steps;
        auto subject =
            std::make_shared<animation>([&steps](double v) { steps.push_back(v); }, 0.0, 1.0, easing::linear());
        subject->update(0.5);
        EXPECT_FALSE(subject->has_finished());
        subject->update(1.0);
        EXPECT_TRUE(subject->has_finished());
        ASSERT_EQ(steps.size(), 2U);
        EXPECT_DOUBLE_EQ(steps[0], 0.5);
        EXPECT_DOUBLE_EQ(steps[1], 1.0);
    }

    TEST(animation, throwing_step_finishes_the_animation)
    {
        auto subject = std::make_shared<animation>([](double) { throw std::runtime_error("boom"); });
        subject->update(0.5);
        EXPECT_TRUE(subject->has_finished());
    }

    TEST(animation, tick_advances_current_time_and_honors_start_delay)
    {
        std::vector<double> steps;
        auto subject = std::make_shared<animation>([&steps](double v) { steps.push_back(v); },
                                                   /*start=*/0.5, /*duration=*/1.0, easing::linear());
        subject->tick(250); // 0.25 s — still inside the delay window
        EXPECT_DOUBLE_EQ(subject->current_time(), 0.25);
        EXPECT_TRUE(steps.empty());
        subject->tick(750); // 1.0 s total — 0.5 s into the run
        ASSERT_EQ(steps.size(), 1U);
        EXPECT_DOUBLE_EQ(steps[0], 0.5);
        subject->tick(500); // 1.5 s total — the end
        EXPECT_TRUE(subject->has_finished());
        EXPECT_DOUBLE_EQ(steps.back(), 1.0);
    }

    TEST(animation, finished_callback_fires_once_at_the_end)
    {
        int finished = 0;
        auto subject =
            std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear(), [&finished] { ++finished; });
        subject->tick(500);
        EXPECT_EQ(finished, 0);
        subject->tick(500);
        EXPECT_EQ(finished, 1);
        subject->tick(500); // already finished — on_tick returns immediately
        EXPECT_EQ(finished, 1);
    }

    TEST(animation, paused_animation_ignores_ticks_and_resume_continues)
    {
        std::vector<double> steps;
        auto subject =
            std::make_shared<animation>([&steps](double v) { steps.push_back(v); }, 0.0, 1.0, easing::linear());
        subject->pause();
        EXPECT_TRUE(subject->is_paused());
        subject->tick(500);
        EXPECT_TRUE(steps.empty());
        subject->resume();
        EXPECT_FALSE(subject->is_paused());
        subject->tick(500);
        ASSERT_EQ(steps.size(), 1U);
        EXPECT_DOUBLE_EQ(steps[0], 0.5);
    }

    TEST(animation, composite_finishes_when_all_children_finish)
    {
        auto fast = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        auto slow = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        auto parent = std::make_shared<animation>();
        parent->add(0.0, 0.5, fast); // child window [0, 0.5)
        parent->add(0.0, 1.0, slow); // child window [0, 1)
        EXPECT_EQ(parent->children().size(), 2U);

        parent->tick(600); // 0.6 s: fast finished, slow not
        EXPECT_TRUE(fast->has_finished());
        EXPECT_FALSE(slow->has_finished());
        EXPECT_FALSE(parent->has_finished());

        parent->tick(400); // 1.0 s: every child finished
        EXPECT_TRUE(slow->has_finished());
        EXPECT_TRUE(parent->has_finished());
    }

    TEST(animation, add_validates_the_window_like_the_oracle)
    {
        auto parent = std::make_shared<animation>();
        const auto child = std::make_shared<animation>();
        EXPECT_THROW(parent->add(-0.1, 0.5, child), std::out_of_range);    // beginAt out of range
        EXPECT_THROW(parent->add(1.1, 0.5, child), std::out_of_range);     // beginAt out of range
        EXPECT_THROW(parent->add(0.0, 1.5, child), std::out_of_range);     // duration out of range
        EXPECT_THROW(parent->add(0.5, 0.5, child), std::invalid_argument); // duration <= beginAt
        parent->add(0.25, 0.75, child);
        EXPECT_DOUBLE_EQ(child->start_delay(), 0.25);
        EXPECT_DOUBLE_EQ(child->duration(), 0.75);
    }

    TEST(animation, reset_rewinds_this_and_every_child)
    {
        auto child = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        auto parent = std::make_shared<animation>();
        parent->add(0.0, 1.0, child);
        parent->tick(1000);
        EXPECT_TRUE(parent->has_finished());
        parent->reset();
        EXPECT_FALSE(parent->has_finished());
        EXPECT_EQ(parent->current_time(), 0.0);
        EXPECT_FALSE(child->has_finished());
        EXPECT_EQ(child->current_time(), 0.0);
    }

    TEST(animation, repeats_resets_after_finishing)
    {
        int finished = 0;
        auto subject =
            std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear(), [&finished] { ++finished; });
        subject->set_repeats(true);
        subject->tick(1000);
        EXPECT_EQ(finished, 1);
        EXPECT_FALSE(subject->has_finished()); // reset for the next lap
        EXPECT_EQ(subject->current_time(), 0.0);
        subject->tick(1000);
        EXPECT_EQ(finished, 2);
    }

    TEST(animation, force_finish_jumps_to_the_end_state)
    {
        double last = -1;
        auto subject = std::make_shared<animation>([&last](double v) { last = v; }, 0.0, 1.0, easing::linear());
        subject->tick(250);
        EXPECT_DOUBLE_EQ(last, 0.25);
        subject->force_finish();
        EXPECT_DOUBLE_EQ(last, 1.0);
        EXPECT_TRUE(subject->has_finished());
    }

    TEST(animation, create_auto_reversing_builds_the_forward_plus_reverse_composite)
    {
        auto forward = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        forward->set_repeats(true);
        const auto reversing = forward->create_auto_reversing();
        // The original's Repeats moves onto the composite.
        EXPECT_TRUE(reversing->repeats());
        EXPECT_FALSE(forward->repeats());
        ASSERT_EQ(reversing->children().size(), 2U);
        EXPECT_EQ(reversing->children()[0].get(), forward.get());
        const auto& reversed = reversing->children()[1];
        // CreateReverse: same duration/easing, start delay shifted past the forward window.
        EXPECT_DOUBLE_EQ(reversed->duration(), 1.0);
        EXPECT_DOUBLE_EQ(reversed->start_delay(), 1.0);
        EXPECT_DOUBLE_EQ(reversing->duration(), reversed->start_delay() + reversed->duration());
    }

    TEST(animation, dispose_detaches_children_and_callbacks)
    {
        int finished = 0;
        auto child = std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear());
        auto subject =
            std::make_shared<animation>([](double) {}, 0.0, 1.0, easing::linear(), [&finished] { ++finished; });
        subject->add(0.0, 1.0, child);
        subject->dispose();
        EXPECT_TRUE(subject->is_disposed());
        EXPECT_TRUE(child->is_disposed());
        EXPECT_TRUE(subject->children().empty());
        subject->tick(2000); // a disposed animation has no step/finished callbacks left
        EXPECT_EQ(finished, 0);
    }

    // ---- lerping_animation ----

    TEST(lerping_animation, lerps_double_values_and_raises_value_changed)
    {
        int changes = 0;
        auto subject = std::make_shared<lerping_animation>([](double) {}, 0.0, 1.0, easing::linear());
        subject->set_start_value(std::any{0.0});
        subject->set_end_value(std::any{10.0});
        subject->set_value_changed([&changes] { ++changes; });
        subject->update(0.25);
        EXPECT_DOUBLE_EQ(std::any_cast<double>(subject->current_value()), 2.5);
        subject->update(1.0);
        EXPECT_DOUBLE_EQ(std::any_cast<double>(subject->current_value()), 10.0);
        EXPECT_EQ(changes, 2);
        EXPECT_TRUE(subject->has_finished());
    }

    TEST(lerping_animation, applies_its_easing_to_the_lerp)
    {
        auto subject = std::make_shared<lerping_animation>([](double) {}, 0.0, 1.0, easing::cubic_in());
        subject->set_start_value(std::any{0.0});
        subject->set_end_value(std::any{8.0});
        subject->update(0.5); // cubic-in(0.5) = 0.125 -> 1.0
        EXPECT_DOUBLE_EQ(std::any_cast<double>(subject->current_value()), 1.0);
    }

    TEST(lerping_animation, missing_values_produce_no_current_value)
    {
        auto subject = std::make_shared<lerping_animation>([](double) {}, 0.0, 1.0, easing::linear());
        subject->update(0.5);
        EXPECT_FALSE(subject->current_value().has_value());
        EXPECT_EQ(subject->current_lerp(), nullptr); // no start/end type to resolve against
    }

    TEST(lerping_animation, unregistered_types_toggle_half_way)
    {
        struct opaque
        {
            int tag;
        };
        auto subject = std::make_shared<lerping_animation>([](double) {}, 0.0, 1.0, easing::linear());
        subject->set_start_value(std::any{opaque{1}});
        subject->set_end_value(std::any{opaque{2}});
        subject->update(0.25);
        EXPECT_EQ(std::any_cast<opaque>(subject->current_value()).tag, 1);
        subject->update(0.75);
        EXPECT_EQ(std::any_cast<opaque>(subject->current_value()).tag, 2);
    }
} // namespace
