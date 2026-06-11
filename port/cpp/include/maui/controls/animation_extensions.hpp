#pragma once
// maui::controls animation extensions  <=  Microsoft.Maui.Controls.AnimationExtensions
//
// The named-animation pump over IAnimatable: animate() runs a (transformed, eased) callback under a
// string handle on an element, abort_animation() cancels by handle, animation_is_running() queries,
// animate_kinetic() runs the velocity/drag variant, and interpolate() builds the C# Interpolate
// lambda. C# extension methods become free functions (PROFILE §5) over `element` — the port's
// animatable base (the batch hooks live there; see element.hpp's W1-14 block). Ported from
// src/Controls/src/Core/AnimationExtensions.cs (+ AnimatableKey.cs, which collapses to the
// per-element handle since the registry lives ON the element — element_animations).
//
// DEVIATIONS (documented):
//   - C#'s generic Animate<T> is ported at its only instantiation shape (double): the transform is a
//     double -> double function (C# routes every public overload through T = double).
//   - C#'s DoAction dispatcher marshalling is omitted: per PROFILE §8 the visual tree (and thus the
//     animation API) is driven on the UI thread only.
//   - The Animate(self, …) null-`self` guard is unrepresentable (references); the transform/callback
//     guards throw std::invalid_argument like the C# ArgumentNullException cases.
//
// The animation manager is resolved like C# GetAnimationManager: walk this element then its logical
// parents for a handler whose maui_context can supply the i_animation_manager service; no context
// anywhere throws std::invalid_argument (C# ArgumentException).

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "maui/animations/easing.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/controls/animation.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls
{
    class element;

    // C# Interpolate(start, end, reverseVal, reverse): x => start + (target - start) * x.
    [[nodiscard]] std::function<double(double)> interpolate(double start, double end = 1.0, double reverse_val = 0.0,
                                                            bool reverse = false);

    // C# internal ViewExtensions.GetAnimationManager(IAnimatable) — public here (the port has no
    // assembly-internal visibility); throws std::invalid_argument when no context is reachable.
    [[nodiscard]] std::shared_ptr<maui::animations::i_animation_manager> get_animation_manager(element& animatable);

    // C# Animate(self, name, animation, rate, length, easing, finished, repeat): run a (composite)
    // controls animation under `name`; `repeat` returning true rewinds and reruns it.
    void animate(element& self, std::string name, const std::shared_ptr<animation>& animation_to_run,
                 std::uint32_t rate = 16, std::uint32_t length = 250,
                 std::optional<maui::animations::easing> easing_function = {},
                 animation::finished_with_result_fn finished = {}, animation::repeat_fn repeat = {});

    // C# Animate(self, name, callback, rate, length, easing, finished, repeat).
    void animate(element& self, std::string name, animation::step_fn callback, std::uint32_t rate = 16,
                 std::uint32_t length = 250, std::optional<maui::animations::easing> easing_function = {},
                 animation::finished_with_result_fn finished = {}, animation::repeat_fn repeat = {});

    // C# Animate<T>(self, name, transform, callback, …) at T = double — the core every overload
    // funnels into. A null manager resolves through get_animation_manager(self).
    void animate(element& self, std::string name, const std::function<double(double)>& transform,
                 animation::step_fn callback,
                 std::uint32_t rate = 16, std::uint32_t length = 250,
                 std::optional<maui::animations::easing> easing_function = {},
                 animation::finished_with_result_fn finished = {}, animation::repeat_fn repeat = {},
                 std::shared_ptr<maui::animations::i_animation_manager> manager = nullptr);

    // C# AnimateKinetic(self, name, callback(distance, velocity) -> keep-going, velocity, drag,
    // finished, manager): velocity decays by drag each frame; the callback receives the per-frame
    // distance and remaining velocity until it returns false or the velocity reaches zero.
    void animate_kinetic(element& self, std::string name, maui::core::move_only_function<bool(double, double)> callback,
                         double velocity, double drag, maui::core::move_only_function<void()> finished = {},
                         std::shared_ptr<maui::animations::i_animation_manager> manager = nullptr);

    // C# AbortAnimation(self, handle): cancel the named animation (and kinetic); true if one existed.
    // Canceled animations complete with (1.0, canceled: true).
    bool abort_animation(element& self, std::string_view handle);

    // C# AnimationIsRunning(self, handle).
    [[nodiscard]] bool animation_is_running(element& self, std::string_view handle);

    // C# AnimationExtensions.Batch(self): RAII batch — begin on construction, commit on destruction.
    class batch_scope final
    {
    public:
        explicit batch_scope(element& target);
        batch_scope(const batch_scope&) = delete;
        batch_scope& operator=(const batch_scope&) = delete;
        batch_scope(batch_scope&&) = delete;
        batch_scope& operator=(batch_scope&&) = delete;
        ~batch_scope();

    private:
        element* target_;
    };
} // namespace maui::controls
