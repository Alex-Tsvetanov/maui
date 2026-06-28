#pragma once
// maui::command — the view-model command the binding engine wires to Button.Command and gesture
// commands (PUBLIC_API_DESIGN.md §6, `maui::command LoginCommand{[this]{ LoginAsync(); }}`).
//
// A lighter, value-shaped front for the canonical controls::command (Microsoft.Maui.Controls.Command):
// default-constructible so a view-model can declare it as a member and wire it in its constructor, then
// constructed from a parameterless action plus an optional can-execute predicate. The binding layer
// adapts it to controls::i_command when a Button.Command is bound (see maui/xaml_build.hpp).
//
// Non-copyable/non-movable: it owns a can_execute_changed event whose subscribers hold a back-reference,
// so it lives at a stable address as a view-model member (the view-model is non-movable regardless,
// since its maui::property members are).

#include <functional>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/task.hpp"

namespace maui
{
    class command
    {
    public:
        command() = default;
        // From a parameterless action (+ optional can-execute). Implicit so `LoginCommand{[this]{...}}`
        // and `= [this]{...}` read naturally at the view-model declaration site.
        command(std::function<void()> execute, // NOLINT(google-explicit-constructor) — VM ergonomics
                std::function<bool()> can_execute = {})
            : execute_(std::move(execute)), can_execute_(std::move(can_execute))
        {
        }
        command(const command&) = delete;
        command(command&&) = delete;
        command& operator=(const command&) = delete;
        command& operator=(command&&) = delete;
        ~command() = default;

        // ICommand.Execute — run if there is an action (a no-op when unset, matching a null binding).
        void execute() const
        {
            if (execute_)
            {
                execute_();
            }
        }
        // ICommand.CanExecute — the predicate's answer, or true when none was supplied.
        [[nodiscard]] bool can_execute() const
        {
            return can_execute_ ? can_execute_() : true;
        }
        // Command.ChangeCanExecute — signal that can_execute's answer may have changed.
        void change_can_execute()
        {
            can_execute_changed.raise();
        }
        [[nodiscard]] explicit operator bool() const
        {
            return static_cast<bool>(execute_);
        }

        // Raised when can_execute's answer may have changed (subscribers re-query; RAII-token semantics).
        maui::core::event<> can_execute_changed;

    private:
        std::function<void()> execute_;
        std::function<bool()> can_execute_;
    };
} // namespace maui
