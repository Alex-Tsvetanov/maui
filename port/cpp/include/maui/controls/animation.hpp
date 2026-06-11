#pragma once
// maui::controls::animation  <=  Microsoft.Maui.Controls.Animation
//
// The Controls-layer animation: a maui::animations::animation whose default easing is LINEAR, whose
// (callback, start, end) constructor interpolates the callback values across [start, end] (the Core
// base treats them as delay/duration in seconds instead), and whose child windows are expressed as
// [begin_at, finish_at] FRACTIONS of the parent (add/insert/with_concurrent). get_callback() flattens
// the composite into one eased Action<double> — the form the tweener pipeline consumes — and
// commit(owner, ...) runs it on an element through animation_extensions::animate. Ported from
// src/Controls/src/Core/Animation.cs.
//
// Like the base, instances are always shared_ptr-owned (get_callback hands out a closure that shares
// ownership of this animation, so the running tweener keeps it alive).

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "maui/animations/animation.hpp"
#include "maui/animations/easing.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls
{
    class element;

    class animation : public maui::animations::animation
    {
    public:
        // C# Action<double, bool> finished — (final value, was canceled / animations disabled).
        using finished_with_result_fn = maui::core::move_only_function<void(double, bool)>;
        // C# Func<bool> repeat — return true to run another lap.
        using repeat_fn = maui::core::move_only_function<bool()>;

        // C# Animation(): Easing = Linear.
        animation();
        // C# Animation(callback, start = 0, end = 1, easing = null, finished = null): the callback
        // receives values interpolated across [start, end]; null easing => Linear.
        explicit animation(step_fn callback, double start = 0.0, double end = 1.0,
                           std::optional<maui::animations::easing> easing_function = {}, finished_fn finished = {});

        // C# Add(beginAt, finishAt, animation): child window [beginAt, finishAt] as fractions of this
        // animation; both within [0,1] (std::out_of_range) and finishAt > beginAt
        // (std::invalid_argument).
        void add(double begin_at, double finish_at, std::shared_ptr<animation> child);
        // C# Insert: Add, returning this animation for chaining.
        animation& insert(double begin_at, double finish_at, std::shared_ptr<animation> child);
        // C# WithConcurrent(animation, beginAt, finishAt): add WITHOUT the range validation.
        animation& with_concurrent(std::shared_ptr<animation> child, double begin_at = 0.0, double finish_at = 1.0);
        // C# WithConcurrent(callback, start, end, easing, beginAt, finishAt): mint + add the child.
        animation& with_concurrent(step_fn callback, double start = 0.0, double end = 1.0,
                                   std::optional<maui::animations::easing> easing_function = {}, double begin_at = 0.0,
                                   double finish_at = 1.0);

        // C# GetCallback(): one callback that steps this animation (eased) and recursively drives the
        // children that have begun and not finished, firing each child's Finished exactly once.
        [[nodiscard]] step_fn get_callback();

        // C# Commit(owner, name, rate, length, easing, finished, repeat): run on `owner` under the
        // animation handle `name` (16 ms between frames and 250 ms length by default — lengths here
        // are MILLISECONDS, unlike the Core base's seconds). The base's manager-commit stays visible.
        using maui::animations::animation::commit;
        void commit(element& owner, std::string name, std::uint32_t rate = 16, std::uint32_t length = 250,
                    std::optional<maui::animations::easing> easing_function = {}, finished_with_result_fn finished = {},
                    repeat_fn repeat = {});

        // C# IsEnabled: whether the committed manager's ticker is system-enabled (false if never
        // committed).
        [[nodiscard]] bool is_enabled() const;

        // C# Reset override: also clear the finished-triggered latch get_callback maintains.
        void reset() override;

    private:
        bool finished_triggered_ = false;
    };
} // namespace maui::controls
