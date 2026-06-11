#pragma once
// maui::core::evaluate_java_script_request  <=  Microsoft.Maui.EvaluateJavaScriptAsyncRequest
//
// The payload of the "evaluate_java_script" handler command (C#'s
// CommandMapper[nameof(IWebView.EvaluateJavaScriptAsync)]). C# models the round trip as a
// TaskCompletionSource<string> the platform completes (RunAndReport → SetResult); the port follows the
// async idiom established by the image-source pipeline — a one-shot COMPLETION CALLBACK instead of a
// task. The control mints the request with the (escaped + wrapped) script and a completion that
// post-processes the raw platform result; the platform partial calls complete(result) exactly once when
// the evaluation finishes (WKWebView's completionHandler arrives on the main thread; the headless
// partial completes synchronously through its canned-result seam).
//
// complete() is idempotent — the first call wins, later calls are ignored (TaskCompletionSource
// semantics: SetResult on an already-completed source is the error path; the port drops it).
// DEVIATION: C#'s SetCanceled (platform view gone) is surfaced as complete("null") — the same value an
// errored script yields — because the port's callback channel has no faulted state.

#include <string>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class evaluate_java_script_request
    {
    public:
        // Invoked exactly once with the platform's raw evaluation result (the JSON.stringify'd value, or
        // "null" for an errored/void script).
        using completion = move_only_function<void(std::string result)>;

        evaluate_java_script_request(std::string script, completion on_completed)
            : script_(std::move(script)), on_completed_(std::move(on_completed))
        {
        }

        // C# EvaluateJavaScriptAsyncRequest.Script — the JavaScript to be evaluated.
        [[nodiscard]] const std::string& script() const
        {
            return script_;
        }

        // C# RunAndReport/SetResult: deliver the result to the completion (first call wins).
        void complete(std::string result)
        {
            if (completed_ || !on_completed_)
            {
                return;
            }
            completed_ = true;
            // Move the completion out first so a re-entrant complete() from inside it stays a no-op.
            completion callback = std::move(on_completed_);
            on_completed_ = nullptr;
            callback(std::move(result));
        }

        // Whether complete() already ran (C# Task.IsCompleted).
        [[nodiscard]] bool completed() const
        {
            return completed_;
        }

    private:
        std::string script_;
        completion on_completed_;
        bool completed_ = false;
    };
} // namespace maui::core
