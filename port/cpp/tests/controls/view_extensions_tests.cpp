// Tests for the controls animation surface (W1-14): the view extensions (fade_to / scale_to /
// rotate_to / translate_to / layout_to / cancel_animations), the animation-extensions pump
// (animate / animate_kinetic / abort / is-running / batch hooks) and the ticker system-enabled
// force-finish behavior. Ported from src/Controls/tests/Core.UnitTests/{ViewUnitTests.cs (the
// animation cases), MotionTests.cs, TickerSystemEnabledTests.cs} with the C# test rig rebuilt the
// port way: AnimationReadyHandler -> a stub view_handler whose maui_context serves a
// TestAnimationManager (adjust_speed pinned to 16 ms, TestAnimationManager.cs) over the
// deterministic manual_ticker + manual_dispatcher (the BlockingTicker/AsyncTicker analog — instead
// of awaiting Tasks, tests pump dispatcher.advance and read the completion callback).
#include "maui/controls/view_extensions.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/animations/animation_manager.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/manual_ticker.hpp"
#include "maui/controls/animation.hpp"
#include "maui/controls/animation_extensions.hpp"
#include "maui/controls/button.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "src/controls/detail/tweener.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::animations::animation_manager;
    using maui::animations::i_animation_manager;
    using maui::animations::manual_ticker;
    using maui::controls::animation_completion;
    using maui::controls::button;
    using maui::core::manual_dispatcher;

    // C# TestAnimationManager: pin every tick to 16 ms so runs are predictable.
    class test_animation_manager final : public animation_manager
    {
    public:
        using animation_manager::animation_manager;

    protected:
        [[nodiscard]] double adjust_speed(double /*elapsed_milliseconds*/) const override
        {
            return 16;
        }
    };

    // The AnimationReadyMauiContext analog: a context whose services hold the animation manager.
    class test_maui_context final : public maui::core::i_maui_context
    {
    public:
        explicit test_maui_context(std::shared_ptr<i_animation_manager> manager)
        {
            services_.add_singleton<i_animation_manager>(std::move(manager));
        }
        [[nodiscard]] maui::core::service_registry& services() override
        {
            return services_;
        }
        [[nodiscard]] maui::core::handler_registry& handlers() override
        {
            return handlers_;
        }

    private:
        maui::core::service_registry services_;
        maui::core::handler_registry handlers_;
    };

    // The AnimationReadyHandler analog: a no-op handler that only carries the maui_context.
    struct stub_platform
    {
    };
    class animation_ready_handler final
        : public maui::core::view_handler<animation_ready_handler, maui::core::i_view, stub_platform>
    {
    public:
        animation_ready_handler() : view_handler(nullptr, nullptr)
        {
        }
        static std::unique_ptr<stub_platform> create_platform_view()
        {
            return std::make_unique<stub_platform>();
        }
        [[nodiscard]] maui::graphics::size get_desired_size(double /*width*/, double /*height*/) const override
        {
            return {};
        }
        void platform_arrange(const maui::graphics::rect& /*frame*/) override
        {
        }
    };

    struct animation_fixture
    {
        manual_dispatcher dispatcher;
        std::shared_ptr<manual_ticker> ticker = std::make_shared<manual_ticker>(dispatcher);
        std::shared_ptr<test_animation_manager> manager = std::make_shared<test_animation_manager>(ticker);
        test_maui_context context{manager};

        // AnimationReadyHandler.Prepare(view).
        template <class TView> TView& prepare(TView& target)
        {
            auto handler = std::make_shared<animation_ready_handler>();
            handler->set_maui_context(&context);
            target.set_handler(std::move(handler));
            return target;
        }

        void advance(int milliseconds)
        {
            dispatcher.advance(std::chrono::milliseconds(milliseconds));
        }
    };

    // A completion recorder standing in for the C# awaited Task<bool>.
    struct completion_recorder
    {
        bool completed = false;
        bool canceled = false;

        animation_completion callback()
        {
            return [this](bool was_canceled) {
                completed = true;
                canceled = was_canceled;
            };
        }
    };

    // ---- ViewUnitTests (the *To cases) ----

    TEST(view_extensions, fade_to_animates_opacity) // ViewUnitTests.TestFadeTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        completion_recorder done;

        fade_to(view, 0.1, 250, {}, done.callback());
        EXPECT_TRUE(animation_is_running(view, "fade_to"));
        fx.advance(1000);

        EXPECT_NEAR(view.opacity(), 0.1, 0.001);
        EXPECT_TRUE(done.completed);
        EXPECT_FALSE(done.canceled);
        EXPECT_FALSE(animation_is_running(view, "fade_to"));
    }

    TEST(view_extensions, translate_to_animates_both_axes) // ViewUnitTests.TestTranslateTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        completion_recorder done;

        translate_to(view, 100, 50, 250, {}, done.callback());
        fx.advance(1000);

        EXPECT_DOUBLE_EQ(view.translation_x(), 100);
        EXPECT_DOUBLE_EQ(view.translation_y(), 50);
        EXPECT_TRUE(done.completed);
        EXPECT_FALSE(done.canceled);
    }

    TEST(view_extensions, scale_to_animates_scale) // ViewUnitTests.ScaleTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        completion_recorder done;

        scale_to(view, 2, 250, {}, done.callback());
        fx.advance(1000);

        EXPECT_DOUBLE_EQ(view.scale(), 2);
        EXPECT_TRUE(done.completed);
    }

    TEST(view_extensions, rotate_to_animates_rotation) // ViewUnitTests.TestRotateTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        rotate_to(view, 25);
        fx.advance(1000);
        EXPECT_NEAR(view.rotation(), 25, 0.001);
    }

    TEST(view_extensions, rotate_y_to_animates_rotation_y) // ViewUnitTests.TestRotateYTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        rotate_y_to(view, 25);
        fx.advance(1000);
        EXPECT_NEAR(view.rotation_y(), 25, 0.001);
    }

    TEST(view_extensions, rotate_x_to_animates_rotation_x) // ViewUnitTests.TestRotateXTo
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        rotate_x_to(view, 25);
        fx.advance(1000);
        EXPECT_NEAR(view.rotation_x(), 25, 0.001);
    }

    TEST(view_extensions, rel_rotate_to_rotates_from_the_current_value) // ViewUnitTests.TestRelRotateTo
    {
        animation_fixture fx;
        button view;
        view.set_rotation(30);
        fx.prepare(view);

        rel_rotate_to(view, 20);
        fx.advance(1000);
        EXPECT_NEAR(view.rotation(), 50, 0.001);
    }

    TEST(view_extensions, rel_scale_to_scales_from_the_current_value) // ViewUnitTests.TestRelScaleTo
    {
        animation_fixture fx;
        button view;
        view.set_scale(1);
        fx.prepare(view);

        rel_scale_to(view, 1);
        fx.advance(1000);
        EXPECT_NEAR(view.scale(), 2, 0.001);
    }

    TEST(view_extensions, scale_x_and_y_to_animate_the_axis_scales)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        scale_x_to(view, 3);
        scale_y_to(view, 0.5);
        fx.advance(1000);
        EXPECT_DOUBLE_EQ(view.scale_x(), 3);
        EXPECT_DOUBLE_EQ(view.scale_y(), 0.5);
    }

    TEST(view_extensions, layout_to_eases_the_frame_to_the_bounds)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        view.set_frame({0, 0, 100, 100});
        completion_recorder done;

        layout_to(view, maui::graphics::rect{10, 20, 200, 150}, 250, {}, done.callback());
        fx.advance(1000);

        EXPECT_DOUBLE_EQ(view.frame().x, 10);
        EXPECT_DOUBLE_EQ(view.frame().y, 20);
        EXPECT_DOUBLE_EQ(view.frame().width, 200);
        EXPECT_DOUBLE_EQ(view.frame().height, 150);
        EXPECT_TRUE(done.completed);
        EXPECT_FALSE(done.canceled);
    }

    TEST(view_extensions, cancel_animations_aborts_and_reports_canceled)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        completion_recorder done;

        fade_to(view, 0.0, 1000, {}, done.callback());
        fx.advance(160); // partway through
        const double opacity_mid = view.opacity();
        EXPECT_LT(opacity_mid, 1.0);
        EXPECT_GT(opacity_mid, 0.0);

        cancel_animations(view);
        EXPECT_TRUE(done.completed);
        EXPECT_TRUE(done.canceled);
        EXPECT_FALSE(animation_is_running(view, "fade_to"));

        fx.advance(1000); // nothing keeps running
        EXPECT_DOUBLE_EQ(view.opacity(), opacity_mid);
    }

    TEST(view_extensions, restarting_a_named_animation_cancels_the_previous_run)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        completion_recorder first;
        completion_recorder second;

        fade_to(view, 0.0, 1000, {}, first.callback());
        fade_to(view, 0.5, 250, {}, second.callback()); // same handle -> aborts the first
        EXPECT_TRUE(first.completed);
        EXPECT_TRUE(first.canceled);

        fx.advance(1000);
        EXPECT_NEAR(view.opacity(), 0.5, 0.001);
        EXPECT_TRUE(second.completed);
        EXPECT_FALSE(second.canceled);
    }

    TEST(view_extensions, animating_without_a_reachable_manager_throws)
    {
        button view; // no handler, no parents -> no maui_context anywhere
        EXPECT_THROW(fade_to(view, 0.5), std::invalid_argument);
    }

    TEST(view_extensions, updates_run_inside_a_batch) // IAnimatable BatchBegin/BatchCommit hooks
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        int commits = 0;
        view.batch_committed.connect([&commits] { ++commits; });

        fade_to(view, 0.0, 250);
        fx.advance(1000);

        EXPECT_GT(commits, 0);
        EXPECT_FALSE(view.batched()); // every begin was committed
    }

    // ---- MotionTests ----

    TEST(motion, test_linear_tween) // MotionTests.TestLinearTween
    {
        animation_fixture fx;
        maui::controls::detail::tweener tween(250, fx.manager);

        double value = 0;
        int updates = 0;
        tween.value_updated.connect([&] {
            EXPECT_GE(tween.value(), value);
            value = tween.value();
            ++updates;
        });
        tween.start();
        fx.advance(2000);

        EXPECT_GE(updates, 10);
    }

    TEST(motion, throws_with_null_callback) // MotionTests.ThrowsWithNullCallback
    {
        button view;
        EXPECT_THROW(animate(view, "Test", maui::controls::animation::step_fn{}), std::invalid_argument);
    }

    TEST(motion, throws_with_null_transform) // MotionTests.ThrowsWithNullTransform
    {
        button view;
        EXPECT_THROW(animate(view, "Test", std::function<double(double)>{}, [](double) {}), std::invalid_argument);
    }

    TEST(motion, kinetic) // MotionTests.Kinetic
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        std::vector<std::pair<double, double>> results;
        animate_kinetic(
            view, "kinetics",
            [&results](double distance, double velocity) {
                results.emplace_back(distance, velocity);
                return true;
            },
            /*velocity=*/100, /*drag=*/1);
        fx.advance(2000);

        ASSERT_FALSE(results.empty());
        double check_velocity = 100;
        constexpr double drag_step = 16;
        for (const auto& [distance, velocity] : results)
        {
            check_velocity -= drag_step;
            EXPECT_DOUBLE_EQ(velocity, check_velocity);
            EXPECT_DOUBLE_EQ(distance, check_velocity * drag_step);
        }
    }

    TEST(motion, kinetic_finished) // MotionTests.KineticFinished
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        bool finished = false;
        animate_kinetic(
            view, "kinetics", [](double, double) { return true; }, /*velocity=*/100, /*drag=*/1,
            [&finished] { finished = true; });
        fx.advance(2000);

        EXPECT_TRUE(finished);
    }

    TEST(motion, insert_and_with_concurrent_compose_children) // Animation.Insert / WithConcurrent
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);

        double first = -1;
        double second = -1;
        auto composite = std::make_shared<maui::controls::animation>();
        composite->insert(0, 1, std::make_shared<maui::controls::animation>([&first](double v) { first = v; }, 0, 1))
            .with_concurrent([&second](double v) { second = v; }, 0, 2);
        composite->commit(view, "composite", 16, 250);
        fx.advance(1000);

        EXPECT_DOUBLE_EQ(first, 1.0);
        EXPECT_DOUBLE_EQ(second, 2.0);
    }

    TEST(motion, abort_animation_returns_false_when_nothing_is_running)
    {
        button view;
        EXPECT_FALSE(abort_animation(view, "nothing"));
    }

    // ---- TickerSystemEnabledTests ----

    TEST(ticker_system_enabled, disabling_ticker_finishes_animation_in_progress)
    {
        animation_fixture fx;
        button view;
        view.set_opacity(1);
        fx.prepare(view);
        completion_recorder done;

        fade_to(view, 0, 2000, {}, done.callback());
        fx.advance(64); // a few frames in
        fx.ticker->set_system_enabled(false);

        EXPECT_DOUBLE_EQ(view.opacity(), 0);
        EXPECT_TRUE(done.completed);
        EXPECT_TRUE(done.canceled); // C#: finished(value, !animationsEnabled)
    }

    TEST(ticker_system_enabled, disabling_ticker_finishes_all_animations_in_chain)
    {
        animation_fixture fx;
        button view1;
        button view2;
        view1.set_opacity(1);
        view2.set_opacity(0);
        fx.prepare(view1);
        fx.prepare(view2);

        // SwapFadeViews: the second fade starts when the first completes.
        button* second = &view2;
        fade_to(view1, 0, 15000, {}, [second](bool) { fade_to(*second, 1, 15000, {}, animation_completion{}); });
        fx.advance(64);
        fx.ticker->set_system_enabled(false);

        EXPECT_DOUBLE_EQ(view1.opacity(), 0);
        EXPECT_DOUBLE_EQ(view2.opacity(), 1);
    }

    TEST(ticker_system_enabled, disabling_ticker_prevents_animation_from_repeating)
    {
        animation_fixture fx;
        button view;
        view.set_opacity(0);
        fx.prepare(view);

        // RepeatFade: a 1-second fade repeated 5 times; disabling the ticker finishes it immediately.
        button* target = &view;
        auto fade_in =
            std::make_shared<maui::controls::animation>([target](double d) { target->set_opacity(d); }, 0, 1);
        int laps = 0;
        bool completed = false;
        bool completed_canceled = false;
        fade_in->commit(
            view, "fade_in", 16, 1000, std::nullopt,
            [&laps, &completed, &completed_canceled](double, bool canceled) {
                if ((canceled || laps >= 5) && !completed)
                {
                    completed = true;
                    completed_canceled = canceled;
                }
            },
            [&laps] { return ++laps < 5; });

        fx.advance(160); // partway through the first lap
        fx.ticker->set_system_enabled(false);

        EXPECT_DOUBLE_EQ(view.opacity(), 1);
        EXPECT_TRUE(completed);
        EXPECT_TRUE(completed_canceled);
        EXPECT_LT(laps, 5); // it never ran all five laps
    }

    TEST(ticker_system_enabled, new_animations_finish_immediately_when_ticker_disabled)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        fx.ticker->set_system_enabled(false);
        completion_recorder done;

        rotate_y_to(view, 200, 250, {}, done.callback());

        EXPECT_DOUBLE_EQ(view.rotation_y(), 200); // synchronously — no time pumped
        EXPECT_TRUE(done.completed);
        EXPECT_TRUE(done.canceled);
    }

    TEST(ticker_system_enabled, animation_extensions_return_true_if_animations_disabled)
    {
        animation_fixture fx;
        button view;
        fx.prepare(view);
        fx.ticker->set_system_enabled(false);
        completion_recorder done;

        scale_to(view, 2, 500, {}, done.callback());

        EXPECT_TRUE(done.completed);
        EXPECT_TRUE(done.canceled); // the C# Task<bool> resolves true when animations are disabled
        EXPECT_DOUBLE_EQ(view.scale(), 2);
    }

    // ---- teardown safety (the port's deterministic-ownership deviation) ----

    TEST(view_extensions, destroying_the_view_detaches_its_running_animations)
    {
        animation_fixture fx;
        {
            button view;
            fx.prepare(view);
            fade_to(view, 0.0, 10000);
            animate_kinetic(view, "kinetics", [](double, double) { return true; }, 100, 0.0001);
            EXPECT_TRUE(fx.ticker->is_running());
        } // the view dies mid-animation
        // The manager was emptied by the teardown; pumping must not touch freed memory and the
        // ticker stops on the next fire (no animations remain).
        fx.advance(64);
        EXPECT_FALSE(fx.ticker->is_running());
    }
} // namespace
