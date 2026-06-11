#pragma once
// maui::controls::detail::element_animations — the per-element slice of C# AnimationExtensions'
// static tables (s_animations keyed by AnimatableKey(self, handle), s_kinetics) plus the per-run
// bookkeeping record (C# AnimationExtensions.Info). Internal (PROFILE §3).
//
// C# keys process-wide static dictionaries by a weak (animatable, handle) pair and leans on
// finalizers for cleanup; every access goes through `self`, so the port stores the same state ON the
// element (element::animation_state) — no global mutable state, and destroying the element
// deterministically detaches its running animations from the manager WITHOUT raising completion
// callbacks (in C# a dropped view's animation keeps ticking against dead weak references until it
// ends naturally; the port documents this sharper teardown as the §8-conform deviation).

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "maui/animations/easing.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/core/move_only_function.hpp"
#include "tweener.hpp"

namespace maui::controls::detail
{
    // C# AnimationExtensions.Info — one named animation run.
    struct animation_info
    {
        std::shared_ptr<tweener> tween;                              // C# Info.Tweener
        maui::core::move_only_function<void(double)> callback;       // C# Info.Callback
        maui::core::move_only_function<void(double, bool)> finished; // C# Info.Finished
        maui::core::move_only_function<bool()> repeat;               // C# Info.Repeat
        maui::animations::easing easing_function{maui::animations::easing::linear()};
        std::shared_ptr<maui::animations::i_animation_manager> manager; // C# Info.AnimationManager
        std::uint32_t rate = 16;
        std::uint32_t length = 250;
    };

    class element_animations final
    {
    public:
        element_animations() = default;
        element_animations(const element_animations&) = delete;
        element_animations& operator=(const element_animations&) = delete;
        element_animations(element_animations&&) = delete;
        element_animations& operator=(element_animations&&) = delete;
        // Silent detach: the named animations' tweeners remove themselves from the manager in their
        // destructors; the kinetic animations (manager-owned shared_ptrs) are removed explicitly so
        // their steps — which reference the dying element — can never tick again.
        ~element_animations();

        std::unordered_map<std::string, animation_info> animations;                   // s_animations slice
        std::unordered_map<std::string, std::shared_ptr<tweener_animation>> kinetics; // s_kinetics slice
    };
} // namespace maui::controls::detail
