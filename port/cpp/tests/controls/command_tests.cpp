// Command primitive tests (headless) — the U-CMD i_command / command unit. Ports the portable subset of
// the C# behavioral oracle src/Controls/tests/Core.UnitTests/CommandTests.cs:
//   - Constructor / Execute / ExecuteParameterized / ExecuteWithCanExecute / CanExecute(true|false) /
//     ChangeCanExecute, plus the null-execute / null-canExecute ArgumentNullException cases.
// NOT ported (documented in command.hpp): the Command<T> generic (strongly-typed-parameter IsValidParameter
// via reflection-era `is T` / Nullable.GetUnderlyingType) and the WeakReference GC-collection theory
// (CommandsSubscribedToCanExecuteCollect) — both lean on .NET facilities (RTTI on the box, GC) the port
// does not reproduce; the port's leak doctrine is the RAII event token exercised below.

#include <any>
#include <functional>
#include <stdexcept>
#include <string>

#include "maui/controls/command.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::command;

    // CommandTests.Constructor: a no-canExecute command can always execute.
    TEST(command_test, constructor_can_execute_true_with_no_predicate)
    {
        const command cmd{[] {}};
        EXPECT_TRUE(cmd.can_execute(std::any{})); // CanExecute(null)
    }

    // CommandTests.ThrowsWithNullConstructor / ThrowsWithNullParameterizedConstructor.
    TEST(command_test, throws_with_null_execute)
    {
        EXPECT_THROW(command{std::function<void()>{}}, std::invalid_argument);
        EXPECT_THROW(command{std::function<void(const std::any&)>{}}, std::invalid_argument);
    }

    // CommandTests.ThrowsWithNullCanExecute / ThrowsWithNullParameterizedCanExecute /
    // ThrowsWithNullExecuteValidCanExecute.
    TEST(command_test, throws_with_null_can_execute_or_execute_in_two_arg_ctors)
    {
        EXPECT_THROW(command(std::function<void()>{[] {}}, std::function<bool()>{}), std::invalid_argument);
        EXPECT_THROW(command(std::function<void(const std::any&)>{[](const std::any&) {}},
                             std::function<bool(const std::any&)>{}),
                     std::invalid_argument);
        EXPECT_THROW(command(std::function<void()>{}, std::function<bool()>{[] { return true; }}),
                     std::invalid_argument);
    }

    // CommandTests.Execute: the parameterless action runs.
    TEST(command_test, execute_runs_parameterless_action)
    {
        bool executed = false;
        command cmd{[&executed] { executed = true; }};
        cmd.execute(std::any{});
        EXPECT_TRUE(executed);
    }

    // CommandTests.ExecuteParameterized: the parameter reaches the action.
    TEST(command_test, execute_passes_parameter)
    {
        std::any executed;
        command cmd{[&executed](const std::any& o) { executed = o; }};

        cmd.execute(std::any{std::string("expected")});
        ASSERT_TRUE(executed.has_value());
        EXPECT_EQ(std::any_cast<std::string>(executed), "expected");
    }

    // CommandTests.ExecuteWithCanExecute: execute still runs when a (true) predicate is present.
    TEST(command_test, execute_runs_with_can_execute_present)
    {
        bool executed = false;
        command cmd{std::function<void()>{[&executed] { executed = true; }},
                    std::function<bool()>{[] { return true; }}};
        cmd.execute(std::any{});
        EXPECT_TRUE(executed);
    }

    // CommandTests.CanExecute([true]/[false]): the predicate's answer is returned and the predicate ran.
    TEST(command_test, can_execute_returns_predicate_result)
    {
        for (const bool expected : {true, false})
        {
            bool can_execute_ran = false;
            const command cmd{std::function<void()>{[] {}}, std::function<bool()>{[&can_execute_ran, expected] {
                                  can_execute_ran = true;
                                  return expected;
                              }}};
            EXPECT_EQ(cmd.can_execute(std::any{}), expected);
            EXPECT_TRUE(can_execute_ran);
        }
    }

    // The parameterized predicate receives the parameter (the Command(Action<object>, Func<object,bool>)
    // path — the gating the gesture recognizers rely on).
    TEST(command_test, parameterized_can_execute_receives_parameter)
    {
        std::any seen;
        const command cmd{[](const std::any&) {},
                          [&seen](const std::any& o) {
                              seen = o;
                              return true;
                          }};
        EXPECT_TRUE(cmd.can_execute(std::any{42}));
        ASSERT_TRUE(seen.has_value());
        EXPECT_EQ(std::any_cast<int>(seen), 42);
    }

    // CommandTests.ChangeCanExecute: change_can_execute raises can_execute_changed.
    TEST(command_test, change_can_execute_raises_event)
    {
        bool signaled = false;
        command cmd{[] {}};
        cmd.can_execute_changed.connect([&signaled] { signaled = true; });

        cmd.change_can_execute();
        EXPECT_TRUE(signaled);
    }
} // namespace
