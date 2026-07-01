// Diagnostic tests for the gallery Animations page (examples/gallery/pages/animation_page.hpp),
// which ports AnimationPage.xaml(.cs). The reported parity issue is "no animation over the .NET bot"
// in the C++ port. These tests settle whether that is a PORT CODE bug (the animation subsystem does
// not move the target when the page's buttons are pressed) or a CAPTURE bug (the port animates fine,
// but the parity pipeline only records a single static frame).
//
// The experiment is device-free and decisive: construct the page, attach the AnimationReady handler +
// maui_context (the animation-manager service) onto the animated target exactly the way the C#
// AnimationReadyHandler.Prepare rig does (mirrored from view_extensions_tests.cpp — the port's
// TestAnimationManager pins every tick to 16 ms over a deterministic manual_ticker + manual_dispatcher),
// invoke the button-click handler path, pump the virtual clock, and assert the target's transform
// actually changes over time and lands on the documented values.
//
// If the transforms DO change when driven, the port's animation code is correct and the "no animation"
// is a capture issue (a still frame of a page that only moves after a synthetic click). If they do NOT,
// there is a real port bug to fix in the animation subsystem.
#include "examples/gallery/pages/animation_page.hpp"

#include <algorithm>
#include <chrono>
#include <memory>

#include "maui/animations/animation_manager.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/manual_ticker.hpp"
#include "maui/controls/animation_extensions.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/view_extensions.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::animations::animation_manager;
    using maui::animations::i_animation_manager;
    using maui::animations::manual_ticker;
    using maui::core::manual_dispatcher;

    // C# TestAnimationManager: pin every tick to 16 ms so runs are deterministic (TestAnimationManager.cs).
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

    // The AnimationReadyMauiContext analog: a context whose services hold the animation manager (the
    // service the port's hosting layer is meant to register, exactly like C# ConfigureAnimations).
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

    // The AnimationReadyHandler analog: a no-op handler that only carries the maui_context, so a view it
    // is attached to can resolve the animation manager (view_extensions_tests.cpp's stub).
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

    // Fixture: build the page and make its animated target animation-ready, exactly as a hosted page's
    // handler mount would if hosting registered an i_animation_manager (which is the whole point of these
    // tests — the *subsystem* is exercised end-to-end through the real page's click handlers).
    struct page_fixture
    {
        maui::samples::animation_page page;
        manual_dispatcher dispatcher;
        std::shared_ptr<manual_ticker> ticker = std::make_shared<manual_ticker>(dispatcher);
        std::shared_ptr<test_animation_manager> manager = std::make_shared<test_animation_manager>(ticker);
        test_maui_context context{manager};

        page_fixture()
        {
            // Attach the animation-ready handler + context onto the animated target (the dotnet_bot
            // image). get_animation_manager checks the element's own handler first, so this makes the
            // manager reachable from every translate_to / composite commit the page fires.
            auto handler = std::make_shared<animation_ready_handler>();
            handler->set_maui_context(&context);
            page.target().set_handler(std::move(handler));
        }

        void advance(int milliseconds)
        {
            dispatcher.advance(std::chrono::milliseconds(milliseconds));
        }
    };

    // ---- Start Animation: the chained translate_to hops ----------------------------------------------

    // Drive in fine 16 ms steps up to `max_ms`, recording the extremes the target's translation reaches.
    // The chained callbacks start each next hop from inside the manager's tick, so the whole chain runs
    // continuously (a coarse advance would step over intermediate hops); fine stepping samples every frame.
    struct translate_extremes
    {
        double min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    };
    translate_extremes drive_and_sample_translation(page_fixture& fx, maui::controls::image& bot, int max_ms)
    {
        translate_extremes ex;
        for (int t = 0; t < max_ms; t += 16)
        {
            fx.advance(16);
            ex.min_x = std::min(ex.min_x, bot.translation_x());
            ex.max_x = std::max(ex.max_x, bot.translation_x());
            ex.min_y = std::min(ex.min_y, bot.translation_y());
            ex.max_y = std::max(ex.max_y, bot.translation_y());
        }
        return ex;
    }

    TEST(animation_page, start_animation_moves_the_target_through_the_hops_to_the_final_position)
    {
        page_fixture fx;
        auto& bot = fx.page.target();

        // At rest the bot is static (the C# page renders it still until a button is pressed).
        EXPECT_DOUBLE_EQ(bot.translation_x(), 0.0);
        EXPECT_DOUBLE_EQ(bot.translation_y(), 0.0);
        EXPECT_TRUE(fx.page.start_button().is_enabled());
        EXPECT_FALSE(fx.page.cancel_button().is_enabled());

        // Fire the button's click path (what the tap invokes).
        fx.page.start_button().send_clicked();

        // The run took over: Start disabled, Cancel enabled, and a translate_to is live.
        EXPECT_FALSE(fx.page.start_button().is_enabled());
        EXPECT_TRUE(fx.page.cancel_button().is_enabled());
        EXPECT_TRUE(maui::controls::animation_is_running(bot, "translate_to"));

        // A few frames in, the transform is genuinely MOVING toward the first hop's -100 x (heading
        // negative, not yet arrived) — the decisive "the ticker advances and the setter is applied" proof.
        fx.advance(160);
        const double x_early = bot.translation_x();
        EXPECT_LT(x_early, 0.0);
        EXPECT_GT(x_early, -100.0);

        // Drive the rest of the chain frame-by-frame. The five hops sweep the target to
        // (-100,0) -> (-100,-100) -> (100,100) -> (0,100) -> (0,0), so the translation must reach the far
        // negative x/y of the early hops AND the far positive x/y of the middle hop before returning.
        const translate_extremes ex = drive_and_sample_translation(fx, bot, 8000);
        EXPECT_LT(ex.min_x, -95.0); // reached hop1/2's -100 x
        EXPECT_LT(ex.min_y, -95.0); // reached hop2's -100 y
        EXPECT_GT(ex.max_x, 95.0);  // reached hop3's +100 x
        EXPECT_GT(ex.max_y, 95.0);  // reached hop3/4's +100 y

        // Landed on the final hop (0, 0), the run finished, and the rest button-state is restored.
        EXPECT_DOUBLE_EQ(bot.translation_x(), 0.0);
        EXPECT_DOUBLE_EQ(bot.translation_y(), 0.0);
        EXPECT_TRUE(fx.page.start_button().is_enabled());
        EXPECT_FALSE(fx.page.cancel_button().is_enabled());
        EXPECT_FALSE(maui::controls::animation_is_running(bot, "translate_to"));
    }

    // ---- Start Custom Animation: the composite scale/rotate run --------------------------------------

    TEST(animation_page, custom_animation_scales_and_rotates_then_returns)
    {
        page_fixture fx;
        auto& bot = fx.page.target();

        EXPECT_DOUBLE_EQ(bot.scale(), 1.0);
        EXPECT_DOUBLE_EQ(bot.rotation(), 0.0);

        fx.page.custom_button().send_clicked();
        EXPECT_TRUE(maui::controls::animation_is_running(bot, "custom_animation"));

        // Drive the 4000 ms composite frame-by-frame, tracking the extremes. The scale child springs up to
        // 2.0 at the midpoint (SpringIn dips slightly below 1.0 first, then overshoots) and the SpringOut
        // brings it back; the rotate child sweeps 0 -> 360 monotonically. Sampling proves BOTH children
        // are actually ticking, not just that the endpoints happen to match.
        double max_scale = bot.scale();
        double max_rotation = bot.rotation();
        double prev_rotation = bot.rotation();
        bool rotation_monotonic = true;
        for (int t = 0; t < 5000; t += 16)
        {
            fx.advance(16);
            max_scale = std::max(max_scale, bot.scale());
            max_rotation = std::max(max_rotation, bot.rotation());
            if (bot.rotation() + 1e-9 < prev_rotation)
            {
                rotation_monotonic = false;
            }
            prev_rotation = bot.rotation();
        }

        EXPECT_GT(max_scale, 1.9);       // the scale child sprang up to ~2.0 at the midpoint
        EXPECT_GT(max_rotation, 359.0);  // the rotate child swept all the way to 360
        EXPECT_TRUE(rotation_monotonic); // rotation only ever advanced -> it was genuinely animating

        // Over-pumped to completion: SpringOut returned scale to 1.0 and rotation landed on 360.
        EXPECT_NEAR(bot.scale(), 1.0, 1e-6);
        EXPECT_NEAR(bot.rotation(), 360.0, 1e-6);
        EXPECT_FALSE(maui::controls::animation_is_running(bot, "custom_animation"));
        EXPECT_TRUE(fx.page.start_button().is_enabled());
        EXPECT_FALSE(fx.page.cancel_button().is_enabled());
    }

    // ---- Cancel Animation: halts an in-flight run ----------------------------------------------------

    TEST(animation_page, cancel_animation_halts_an_in_flight_translate_run)
    {
        page_fixture fx;
        auto& bot = fx.page.target();

        fx.page.start_button().send_clicked();
        fx.advance(160); // partway through the first hop
        const double x_at_cancel = bot.translation_x();
        EXPECT_LT(x_at_cancel, 0.0);
        EXPECT_GT(x_at_cancel, -100.0);

        // Press Cancel — the C# ViewExtensions.CancelAnimations(target) path.
        fx.page.cancel_button().send_clicked();
        EXPECT_FALSE(maui::controls::animation_is_running(bot, "translate_to"));
        EXPECT_TRUE(fx.page.start_button().is_enabled());
        EXPECT_FALSE(fx.page.cancel_button().is_enabled());

        // Pumping more time does NOT move the target any further: the chain was truly halted (the
        // cancel-mid-chain gate stops the remaining hops).
        fx.advance(20000);
        EXPECT_DOUBLE_EQ(bot.translation_x(), x_at_cancel);
    }
} // namespace
