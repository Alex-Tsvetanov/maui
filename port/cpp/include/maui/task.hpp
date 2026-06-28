#pragma once
// maui::task — a minimal fire-and-forget coroutine return type for view-model async handlers
// (PUBLIC_API_DESIGN.md §6, `maui::task LoginAsync() { ... co_return; }`).
//
// Eager-start (initial_suspend = suspend_never) and self-destroying (final_suspend = suspend_never): the
// body runs as soon as the handler is called and the frame is freed at co_return, so no handle is kept
// and nothing leaks. A UI command handler has nowhere to surface a thrown exception, so the contract is
// that the view-model catches internally and routes failures into its own state (e.g. ErrorMessage);
// an escaped exception calls std::terminate, matching a UI thread's unhandled-exception fate.
//
// This is intentionally NOT a general-purpose awaitable/scheduler — it is the smallest type that makes
// `co_return` (and co_await of already-ready awaitables) compile in a view-model method.

#include <coroutine>
#include <exception>

namespace maui
{
    class task
    {
    public:
        struct promise_type
        {
            // NOLINTNEXTLINE(readability-convert-member-functions-to-static) — coroutine ABI hooks.
            task get_return_object() noexcept
            {
                return task{};
            }
            std::suspend_never initial_suspend() noexcept
            {
                return {};
            }
            std::suspend_never final_suspend() noexcept
            {
                return {};
            }
            void return_void() noexcept
            {
            }
            [[noreturn]] void unhandled_exception() noexcept
            {
                std::terminate();
            }
        };
    };
} // namespace maui
