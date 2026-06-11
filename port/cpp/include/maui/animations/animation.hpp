#pragma once
// maui::animations::animation  <=  Microsoft.Maui.Animations.Animation
//
// Represents an animation: a leaf drives one callback (step) across a [start_delay, start_delay +
// duration) window in SECONDS, applying an easing to the progress; a composite ticks a list of child
// animations and finishes when all of them have. Ported from src/Core/src/Animations/Animation.cs.
//
// Ownership (PROFILE §8): animations are ALWAYS shared_ptr-owned (enable_shared_from_this) — the
// manager keeps a shared_ptr to every committed animation, a parent shares ownership of its children
// (C# child lists hold strong refs; create_reverse even shares the same child instances between the
// forward and reversed composites, which shared_ptr models directly), and commit() stores only a
// WEAK back-reference to the manager (the C# field is a strong GC ref; weak avoids the manager <->
// animation retain cycle and any dangling manager access after the service is torn down).
//
// Threading: C# guards Tick with an Interlocked re-entrancy latch because its tickers fire on timer
// threads. The port's tickers fire on the dispatcher thread (PROFILE §8 — UI mutation is
// single-threaded), so the latch is a plain bool with the same skipped-milliseconds catch-up
// semantics. C# IDisposable maps to the explicit dispose() (the destructor needs no manager
// bookkeeping: an animation still referenced by the manager cannot be destroyed).

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/animations/easing.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/i_animator.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::animations
{
    class animation : public std::enable_shared_from_this<animation>
    {
    public:
        using step_fn = maui::core::move_only_function<void(double)>;
        using finished_fn = maui::core::move_only_function<void()>;

        // C# Animation().
        animation();
        // C# Animation(Action<double> callback, double start = 0, double duration = 1, Easing?
        // easing = null, Action? finished = null) — `start` is the delay in seconds before the
        // animation starts; `duration` how long it runs; null easing => Easing.Default (CubicInOut).
        explicit animation(step_fn callback, double start = 0.0, double duration = 1.0,
                           std::optional<easing> easing_function = {}, finished_fn finished = {});
        // C# Animation(List<Animation>) — a composite of child animations.
        explicit animation(std::vector<std::shared_ptr<animation>> animations);

        animation(const animation&) = delete;
        animation& operator=(const animation&) = delete;
        animation(animation&&) = delete;
        animation& operator=(animation&&) = delete;
        virtual ~animation() = default;

        // ---- the C# properties ----
        [[nodiscard]] const std::string& name() const;
        void set_name(std::string value);
        [[nodiscard]] double start_delay() const; // seconds before the animation starts
        void set_start_delay(double value);
        [[nodiscard]] double duration() const; // seconds the animation runs
        void set_duration(double value);
        [[nodiscard]] double current_time() const; // seconds ticked so far (protected set in C#)
        [[nodiscard]] double progress() const;     // the eased progress of the last update
        [[nodiscard]] const easing& easing_function() const;
        void set_easing_function(easing value);
        [[nodiscard]] bool has_finished() const;
        [[nodiscard]] bool repeats() const; // restart from the beginning when finished
        void set_repeats(bool value);
        [[nodiscard]] bool is_paused() const;
        [[nodiscard]] bool is_disposed() const;
        void set_step(step_fn value);         // C# settable Step
        void set_finished(finished_fn value); // C# settable Finished
        // C# AnimationManager property — the manager this animation was committed to (null if none
        // or if the manager is gone).
        [[nodiscard]] std::shared_ptr<i_animation_manager> animation_manager() const;
        // C# internal Parent — the i_animator this animation is attached to (weak back-reference).
        void set_parent(std::weak_ptr<i_animator> parent);

        // C# GetEnumerator — the child animations.
        [[nodiscard]] const std::vector<std::shared_ptr<animation>>& children() const;

        // C# Add(beginAt, duration, animation): add a child whose window is [beginAt, ...) with the
        // given duration; both must be within [0,1] (std::out_of_range) and duration must be greater
        // than beginAt (std::invalid_argument), mirroring the C# argument exceptions.
        void add(double begin_at, double duration, std::shared_ptr<animation> child);

        // C# Tick(milliseconds): advance by the elapsed milliseconds (no-op while paused; re-entrant
        // calls accumulate into the next tick — the "animation is lagging" path).
        void tick(double milliseconds);

        // C# Update(percent): set the eased progress, invoke the step, and finish at percent == 1.
        // A throwing step finishes the animation (the C# catch-all).
        virtual void update(double percent);

        // C# Commit(IAnimationManager): hand this animation to the manager that will tick it.
        void commit(const std::shared_ptr<i_animation_manager>& manager);

        // C# CreateAutoReversing: a new composite running this animation, then its reverse.
        [[nodiscard]] std::shared_ptr<animation> create_auto_reversing();

        // C# Reset: rewind this animation (and every child) to its initial state.
        virtual void reset();

        // C# Pause / Resume: a paused animation ignores ticks and is removed from (re-added to) its
        // manager.
        void pause();
        void resume();

        // C# RemoveFromParent: detach from the owning i_animator, if any.
        void remove_from_parent();

        // C# internal ForceFinish: jump straight to the end state (used by the manager when the
        // system disables animations mid-flight). Virtual — the controls tweener overrides it.
        virtual void force_finish();

        // C# Dispose: dispose the children, detach from the manager, drop the callbacks.
        void dispose();

        // C# protected OnTick(millisecondsSinceLastUpdate): the actual advance — composites tick
        // children; leaves honor the start-delay window and update() with the clamped percent.
        // Public-facing protected seam in C#; exposed to the manager via tick() only.
    protected:
        virtual void on_tick(double milliseconds_since_last_update);
        // C# protected CreateReverse: the reversed twin (children reversed and SHARED, start delay
        // shifted past this animation's window).
        [[nodiscard]] virtual std::shared_ptr<animation> create_reverse();

        // The C# protected/`protected set` members the derived classes (lerping_animation, the
        // controls animation + tweener) reach directly.
        std::vector<std::shared_ptr<animation>> children_; // C# childrenAnimations
        std::weak_ptr<i_animation_manager> animation_manager_;
        step_fn step_;         // C# Step
        finished_fn finished_; // C# Finished
        easing easing_function_{easing::default_easing()};
        std::string name_;
        double start_delay_ = 0.0;
        double duration_ = 1.0;
        double current_time_ = 0.0;
        double progress_ = 0.0;
        bool has_finished_ = false;
        bool repeats_ = false;

    private:
        std::weak_ptr<i_animator> parent_;
        double skipped_milliseconds_ = 0.0;
        bool paused_ = false;
        bool ticking_ = false;
        bool disposed_ = false;
    };
} // namespace maui::animations
