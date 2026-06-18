#pragma once
// maui::controls::command  <=  Microsoft.Maui.Controls.Command
//
// The concrete i_command MAUI ships: an ICommand that wraps an execute action and an optional
// can_execute predicate. Ported from src/Controls/src/Core/Command.cs (the non-generic `Command`).
//
// Construction mirrors C#'s four ctors, by parameter shape:
//   - command(execute)                         <= Command(Action<object>)            : parameterized run
//   - command(execute, can_execute)            <= Command(Action<object>, Func<object,bool>)
//   - command(parameterless_action)            <= Command(Action)                    : the param is ignored
//   - command(parameterless_action, can_run)   <= Command(Action, Func<bool>)
// The two parameterless overloads adapt to the parameterized stores (C# does the same: `this(o =>
// execute())`), so a parameterless command simply drops the std::any parameter. A null execute (or a
// null can_execute where one is required) throws std::invalid_argument — the port's ArgumentNullException
// analog (Command.cs throws ArgumentNullException; cf. toolbar_item / animation_extensions).
//
// can_execute(p): runs the stored predicate, or returns true when none was supplied (Command.CanExecute).
// change_can_execute(): raises can_execute_changed (Command.ChangeCanExecute → WeakEventManager.HandleEvent).
//
// DEVIATIONS (documented):
//   - The std::any parameter is C#'s `object`; an EMPTY any is C#'s null (boxed_value.hpp convention).
//   - Command<T> (the generic, strongly-typed-parameter subclass with IsValidParameter type checks via
//     `is T` / Nullable.GetUnderlyingType) is NOT ported — it leans on reflection-era runtime type
//     identity the port (no RTTI on the box, PROFILE.md §6) does not reproduce. Callers needing a typed
//     parameter unbox in their execute body via maui::core::try_unbox<T> (boxed_value.hpp).
//   - WeakEventManager is replaced by the port's RAII-token event (PROFILE.md §8); see i_command.hpp.

#include <any>
#include <functional>
#include <stdexcept>
#include <utility>

#include "maui/controls/i_command.hpp"

namespace maui::controls
{
    class command final : public i_command
    {
    public:
        // Command(Action<object> execute): the parameterized run, no can_execute (CanExecute == true).
        explicit command(std::function<void(const std::any&)> execute) : execute_(std::move(execute))
        {
            if (!execute_)
            {
                throw std::invalid_argument("command: execute must not be null"); // C# ArgumentNullException
            }
        }

        // Command(Action<object> execute, Func<object,bool> can_execute).
        command(std::function<void(const std::any&)> execute, std::function<bool(const std::any&)> can_execute)
            : execute_(std::move(execute)), can_execute_(std::move(can_execute))
        {
            if (!execute_)
            {
                throw std::invalid_argument("command: execute must not be null"); // C# ArgumentNullException
            }
            if (!can_execute_)
            {
                throw std::invalid_argument("command: can_execute must not be null"); // C# ArgumentNullException
            }
        }

        // Command(Action execute): parameterless run (the std::any parameter is ignored, as C#'s
        // `this(o => execute())`). require() throws now (construction time) on a null action, matching
        // C#'s ctor-body ArgumentNullException rather than deferring to a crash at execute time.
        explicit command(std::function<void()> execute) : command(adapt_execute(require(std::move(execute))))
        {
        }

        // Command(Action execute, Func<bool> can_execute): parameterless run + parameterless predicate.
        command(std::function<void()> execute, std::function<bool()> can_execute)
            : command(adapt_execute(require(std::move(execute))), adapt_can_execute(require(std::move(can_execute))))
        {
        }

        // ICommand.CanExecute: the predicate's answer, or true when none was supplied.
        [[nodiscard]] bool can_execute(const std::any& parameter) const override
        {
            if (can_execute_)
            {
                return can_execute_(parameter);
            }
            return true;
        }

        // ICommand.Execute: invoke the stored action.
        void execute(const std::any& parameter) override
        {
            execute_(parameter);
        }

        // Command.ChangeCanExecute: signal that can_execute's answer may have changed.
        void change_can_execute()
        {
            can_execute_changed.raise();
        }

    private:
        // The parameterless-overload adapters (C#'s `o => execute()` / `o => can_execute()`). The null
        // checks happen here so the parameterless ctors throw before adapting (matching C#'s ctor-body
        // ArgumentNullException, raised on the original Action/Func, not the wrapped lambda).
        template <class F> static F&& require(F&& f)
        {
            if (!f)
            {
                throw std::invalid_argument("command: execute/can_execute must not be null");
            }
            return std::forward<F>(f);
        }
        static std::function<void(const std::any&)> adapt_execute(std::function<void()> execute)
        {
            return [execute = std::move(execute)](const std::any&) { execute(); };
        }
        static std::function<bool(const std::any&)> adapt_can_execute(std::function<bool()> can_execute)
        {
            return [can_execute = std::move(can_execute)](const std::any&) { return can_execute(); };
        }

        std::function<void(const std::any&)> execute_;
        std::function<bool(const std::any&)> can_execute_;
    };
} // namespace maui::controls
