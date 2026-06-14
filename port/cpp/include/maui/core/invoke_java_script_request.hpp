#pragma once
// maui::core::invoke_java_script_request  <=  Microsoft.Maui.HybridWebViewInvokeJavaScriptRequest
//                                              (+ the InvokeJavaScriptAsync round trip)
//
// The payload of the "invoke_java_script" handler command (C#'s
// HybridWebViewHandler.CommandMapper[nameof(IHybridWebView.InvokeJavaScriptAsync)]). C# models the round
// trip as a TaskCompletionSource<string?> the JavaScript callback completes (the
// "__InvokeJavaScriptCompleted|taskId|result" raw message → HybridWebViewTaskManager.SetTaskCompleted);
// the port follows the async idiom established by web_view's evaluate_java_script_request — a one-shot
// COMPLETION CALLBACK instead of a task.
//
// The control mints the request with the JS method name + the JSON-encoded parameter strings (C#
// JSON-serializes each param value; the port takes them pre-serialized — the caller owns serialization,
// because C++23 has no reflection-driven System.Text.Json). The handler builds
// `window.HybridWebView.__InvokeJavaScript(<taskId>, <methodName>, [<params...>])`, evaluates it, and
// when the matching "__InvokeJavaScriptCompleted" raw message arrives, completes the request with the
// RAW result json string (or nullopt for a null/undefined/void result — C#'s null Task result). A
// "__InvokeJavaScriptFailed" raw message completes with failure (the port surfaces it as nullopt — the
// callback channel has no faulted state; the failed-flag distinguishes it). The caller deserializes the
// raw json to its expected type.
//
// complete()/fail() are idempotent — the first call wins (TaskCompletionSource semantics: a second
// SetResult is the error path; the port drops it).

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    class invoke_java_script_request
    {
    public:
        // Invoked exactly once with the result. The optional holds the RAW JSON result string on success
        // (nullopt for a null/undefined/void result OR a failure); `failed` is true only when the JS
        // invocation threw (the "__InvokeJavaScriptFailed" channel).
        using completion = move_only_function<void(std::optional<std::string> result, bool failed)>;

        // method_name: the global JS function to invoke. param_values: the already-JSON-encoded argument
        // strings (each is spliced verbatim into the args array; an empty vector means no arguments).
        invoke_java_script_request(std::string method_name, std::vector<std::string> param_values,
                                   completion on_completed)
            : method_name_(std::move(method_name)), param_values_(std::move(param_values)),
              on_completed_(std::move(on_completed))
        {
        }

        [[nodiscard]] const std::string& method_name() const
        {
            return method_name_;
        }

        [[nodiscard]] const std::vector<std::string>& param_values() const
        {
            return param_values_;
        }

        // The handler assigns the task id it minted before evaluating the JS, so a later raw-message
        // completion can route back to this request.
        void set_task_id(std::string task_id)
        {
            task_id_ = std::move(task_id);
        }

        [[nodiscard]] const std::string& task_id() const
        {
            return task_id_;
        }

        // C# SetTaskCompleted: deliver a successful result (first call wins).
        void complete(std::optional<std::string> result)
        {
            deliver(std::move(result), /*failed=*/false);
        }

        // C# SetTaskFailed: deliver a failure (first call wins). The result carries the failure detail
        // json when present (the JSInvokeError), else nullopt.
        void fail(std::optional<std::string> detail = std::nullopt)
        {
            deliver(std::move(detail), /*failed=*/true);
        }

        [[nodiscard]] bool completed() const
        {
            return completed_;
        }

    private:
        void deliver(std::optional<std::string> result, bool failed)
        {
            if (completed_ || !on_completed_)
            {
                return;
            }
            completed_ = true;
            completion callback = std::move(on_completed_);
            on_completed_ = nullptr;
            callback(std::move(result), failed);
        }

        std::string method_name_;
        std::vector<std::string> param_values_;
        std::string task_id_;
        completion on_completed_;
        bool completed_ = false;
    };
} // namespace maui::core
