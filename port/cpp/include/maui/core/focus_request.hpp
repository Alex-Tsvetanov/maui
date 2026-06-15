#pragma once
// maui::core::focus_request  <=  Microsoft.Maui.FocusRequest (: RetrievePlatformValueRequest<bool>)
//
// The payload of the IView.Focus command: a request the platform focus mapper fulfills by writing back
// whether the native view actually took focus. Ported from src/Core/src/Primitives/FocusRequest.cs (whose
// IsFocused/Result members wrap RetrievePlatformValueRequest<bool>.TrySetResult). The view's focus()
// invokes the handler's "focus" command with this payload and then reads result(); the mapper calls
// try_set_result(success) — the C++ analog of C#'s request.TrySetResult(platformView.BecomeFirstResponder()).
//
// The result is held behind a shared_ptr so the mapper can write through the const std::any payload the
// command_mapper hands it (commands take `const std::any&`, exactly as C# passes the request object by
// reference and mutates it in place). Default-constructed, the result is "not set" (false).

#include <memory>

namespace maui::core
{
    class focus_request
    {
    public:
        focus_request() = default;

        // C# RetrievePlatformValueRequest.TrySetResult: record the realized focus result (first write wins,
        // matching TrySetResult's "only if not already completed" contract).
        void try_set_result(bool value) const
        {
            if (!*completed_)
            {
                *completed_ = true;
                *result_ = value;
            }
        }

        // C# RetrievePlatformValueRequest.Result (FocusRequest.IsFocused): the realized focus result
        // (false until a mapper sets it).
        [[nodiscard]] bool result() const
        {
            return *result_;
        }

    private:
        // shared so a copy of the request (the std::any boxes by value) still observes the mapper's write.
        std::shared_ptr<bool> result_ = std::make_shared<bool>(false);
        std::shared_ptr<bool> completed_ = std::make_shared<bool>(false);
    };
} // namespace maui::core
