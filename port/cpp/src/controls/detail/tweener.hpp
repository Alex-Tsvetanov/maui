#pragma once
// maui::controls::detail::tweener            <=  Microsoft.Maui.Controls.Tweener (internal)
// maui::controls::detail::tweener_animation  <=  Microsoft.Maui.Controls.TweenerAnimation (internal)
//
// The internal pump under the animation extensions (PROFILE §3: internal-only helpers live under
// src/<layer>/detail/). A tweener normalizes elapsed milliseconds into a [0,1] value over `length`,
// raising value_updated at most once per `rate` ms (and always at the end) and finished when the
// value reaches 1 — it rides the animation manager through a tweener_animation, the raw-step
// animation whose tick simply forwards the elapsed milliseconds.
//
// DEVIATION (documented): C# tracks tweener animations through a process-wide id table
// (AnimationExtensions' s_tweeners + the int Insert/Remove handles) solely because its extension
// methods are stateless; the port's tweener owns its animation directly — identical lifecycle, no
// global registry. C#'s finalizer-based detach becomes the deterministic destructor (PROFILE §8):
// destroying a tweener removes its still-running animation from the manager without raising events.

#include <cstdint>
#include <limits>
#include <memory>

#include "maui/animations/i_animation_manager.hpp"
#include "maui/controls/animation.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls::detail
{
    class tweener_animation final : public maui::controls::animation
    {
    public:
        // The raw step: elapsed milliseconds in, "still running" out. The in-band finish_signal value
        // means "jump to the end" (the system disabled animations).
        using raw_step_fn = maui::core::move_only_function<bool(std::int64_t)>;

        explicit tweener_animation(raw_step_fn step);

        // C# TweenerAnimation.ForceFinish: mark finished and signal the step to jump to the end.
        void force_finish() override;

    protected:
        // C# TweenerAnimation.OnTick: forward the elapsed time; finishing raises Finished.
        void on_tick(double milliseconds_since_last_update) override;

    private:
        raw_step_fn raw_step_;
    };

    class tweener final
    {
    public:
        // C#'s long.MaxValue in-band "finish now" signal.
        static constexpr std::int64_t finish_signal = std::numeric_limits<std::int64_t>::max();

        tweener(std::uint32_t length, std::shared_ptr<maui::animations::i_animation_manager> manager);
        tweener(std::uint32_t length, std::uint32_t rate,
                std::shared_ptr<maui::animations::i_animation_manager> manager);
        tweener(const tweener&) = delete;
        tweener& operator=(const tweener&) = delete;
        tweener(tweener&&) = delete;
        tweener& operator=(tweener&&) = delete;
        // The C# finalizer analog: detach the (possibly still running) animation from the manager.
        ~tweener();

        [[nodiscard]] std::uint32_t length() const
        {
            return length_;
        }
        [[nodiscard]] std::uint32_t rate() const
        {
            return rate_;
        }
        [[nodiscard]] bool loop() const
        {
            return loop_;
        }
        void set_loop(bool value)
        {
            loop_ = value;
        }
        [[nodiscard]] double value() const
        {
            return value_;
        }

        // C# Tweener.Finished / ValueUpdated events.
        maui::core::event<> finished;
        maui::core::event<> value_updated;

        // C# Start: (re)insert the raw-step animation into the manager and kick the ticker; with the
        // system disabled, jump straight to the finished state instead.
        void start();
        // C# Stop: detach and raise finished (the abort path resets the value to 0).
        void stop();
        // C# Pause: detach from the manager without raising anything.
        void pause();

    private:
        [[nodiscard]] bool step(std::int64_t milliseconds);
        void finish_immediately();

        std::shared_ptr<maui::animations::i_animation_manager> manager_;
        std::shared_ptr<tweener_animation> animation_; // C#'s _animationManagerKey, owned directly
        // Liveness token (the manual_dispatcher_timer pattern): raises inside step()/stop() take a
        // weak copy and bail from the post-raise member writes if a handler destroyed this tweener.
        std::shared_ptr<int> life_ = std::make_shared<int>(0);
        std::int64_t last_milliseconds_ = 0;
        std::int64_t frames_ = 0;
        double value_ = 0.0;
        std::uint32_t length_;
        std::uint32_t rate_ = 1;
        bool loop_ = false;
    };
} // namespace maui::controls::detail
