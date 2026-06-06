#pragma once
// maui::core::move_only_function<R(Args...)> — a move-only, type-erased callable.
//
// C++23's std::move_only_function is the intended type (PROFILE.md §5), but the libc++ shipping with
// our toolchain (AppleClang 21 / libc++ 210106) does not yet expose it — <functional> has no
// std::move_only_function and __cpp_lib_move_only_function is undefined, even under
// -fexperimental-library. So the port provides this minimal stand-in with the same contract: owns a
// single move-only callable, is itself move-only, and forwards operator() to it. Heap-erased (no
// small-buffer optimization yet); swap in std::move_only_function once libc++ ships it.

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace maui::core
{
    template <class Signature>
    class move_only_function; // only the R(Args...) specialization is defined

    template <class R, class... Args>
    class move_only_function<R(Args...)>
    {
    public:
        move_only_function() noexcept = default;
        move_only_function(std::nullptr_t) noexcept // implicit, to allow `= nullptr` like the std type
        {
        }

        template <class F>
            requires(!std::is_same_v<std::remove_cvref_t<F>, move_only_function> &&
                     std::is_invocable_r_v<R, std::remove_cvref_t<F> &, Args...>)
        move_only_function(F &&f) // implicit, to allow assigning a lambda like the std type
            : callable_(std::make_unique<model<std::remove_cvref_t<F>>>(std::forward<F>(f)))
        {
        }

        move_only_function(const move_only_function &) = delete;
        move_only_function &operator=(const move_only_function &) = delete;
        move_only_function(move_only_function &&) noexcept = default;
        move_only_function &operator=(move_only_function &&) noexcept = default;
        ~move_only_function() = default;

        R operator()(Args... args) const
        {
            return callable_->invoke(std::forward<Args>(args)...);
        }

        explicit operator bool() const noexcept
        {
            return callable_ != nullptr;
        }

    private:
        struct concept_t
        {
            concept_t() = default;
            concept_t(const concept_t &) = delete;
            concept_t(concept_t &&) = delete;
            concept_t &operator=(const concept_t &) = delete;
            concept_t &operator=(concept_t &&) = delete;
            virtual ~concept_t() = default;
            virtual R invoke(Args... args) = 0;
        };

        template <class F>
        struct model final : concept_t
        {
            explicit model(F fn) : fn_(std::move(fn))
            {
            }
            R invoke(Args... args) override
            {
                return std::invoke(fn_, std::forward<Args>(args)...);
            }
            F fn_;
        };

        std::unique_ptr<concept_t> callable_;
    };
}
