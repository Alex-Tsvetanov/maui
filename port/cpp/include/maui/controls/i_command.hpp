#pragma once
// maui::controls::i_command  <=  System.Windows.Input.ICommand
//
// The command abstraction MAUI binds buttons, gestures, and items to: a thing that can be ASKED whether
// it can run right now (can_execute), can be RUN (execute), and that signals when its can-run answer
// may have changed (can_execute_changed). Ported from the System.Windows.Input.ICommand contract that
// Microsoft.Maui.Controls.Command implements (Command.cs) and that the gesture recognizers, Button,
// MenuItem, etc. hold by reference.
//
// The C# command parameter is `object`; the port's `object` is std::any (the same box the binding
// engine + RadioButton.Value use — see boxed_value.hpp). So can_execute/execute take a const std::any&
// (an EMPTY any is the engine's null, i.e. C#'s `null` parameter).
//
// can_execute_changed is C#'s `event EventHandler CanExecuteChanged`. C# raises it through a
// WeakEventManager (so a long-lived command doesn't root a short-lived subscriber); the port's leak
// doctrine (PROFILE.md §8) is RAII tokens instead — subscribe with maui::core::event::connect and hold
// the token / a scoped_connection, disconnecting in your destructor. The event carries no args (the
// C# signature is EventHandler, sender+EventArgs.Empty, both dropped — the port's events are
// sender-less like every other ported event).

#include <any>

#include "maui/core/event.hpp"

namespace maui::controls
{
    class i_command
    {
    public:
        i_command() = default;
        i_command(const i_command&) = delete;
        i_command(i_command&&) = delete;
        i_command& operator=(const i_command&) = delete;
        i_command& operator=(i_command&&) = delete;
        virtual ~i_command() = default;

        // ICommand.CanExecute(object): whether the command can run with this parameter right now.
        [[nodiscard]] virtual bool can_execute(const std::any& parameter) const = 0;

        // ICommand.Execute(object): run the command with this parameter.
        virtual void execute(const std::any& parameter) = 0;

        // ICommand.CanExecuteChanged: raised when can_execute's answer may have changed (a binding /
        // control re-queries can_execute on each raise to re-evaluate its enabled state).
        maui::core::event<> can_execute_changed;
    };
} // namespace maui::controls
