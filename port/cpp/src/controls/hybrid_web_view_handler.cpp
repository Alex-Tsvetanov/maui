// hybrid_web_view_handler — cross-platform part: the shared mapper tables + ctor (HybridWebViewHandler.cs)
// + the protocol-routing message_received() and the invoke-task minting (HybridWebViewHelper.ProcessRawMessage
// + HybridWebViewTaskManager). The platform recipe (create/connect/disconnect/map_*/measure) lives in the
// per-backend partial (src/platform/headless/hybrid_web_view_handler.cpp;
// src/platform/apple_shared/hybrid_web_view_handler.mm for BOTH Apple backends).

#include "maui/controls/hybrid_web_view_handler.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/command_mapper.hpp"
#include "maui/core/hybrid_web_view_protocol.hpp"
#include "maui/core/i_hybrid_web_view.hpp"
#include "maui/core/invoke_java_script_request.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::controls
{
    // HybridWebViewHandler.Mapper: this cut maps no hybrid-specific properties (DefaultFile/HybridRoot are
    // read by the platform recipe at load time, not per-change; FlowDirection on apple/ios runs through the
    // generic view_mapper). Chained onto the shared view_mapper so the generic IView properties map.
    maui::core::property_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler>& hybrid_web_view_handler::
        mapper()
    {
        static maui::core::property_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler> table{
            maui::core::view_mapper(),
            {},
        };
        return table;
    }

    // HybridWebViewHandler.CommandMapper: SendRawMessage / InvokeJavaScriptAsync / EvaluateJavaScriptAsync.
    // The method name `command_mapper` shadows the template, so the type is qualified in the body.
    maui::core::command_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler>& hybrid_web_view_handler::
        command_mapper()
    {
        static maui::core::command_mapper<maui::core::i_hybrid_web_view, hybrid_web_view_handler> table{
            {"send_raw_message", &hybrid_web_view_handler::map_send_raw_message},
            {"invoke_java_script", &hybrid_web_view_handler::map_invoke_java_script},
            {"evaluate_java_script", &hybrid_web_view_handler::map_evaluate_java_script},
        };
        return table;
    }

    hybrid_web_view_handler::hybrid_web_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    std::string hybrid_web_view_handler::create_invoke_task(
        const std::shared_ptr<maui::core::invoke_java_script_request>& request)
    {
        // HybridWebViewTaskManager.CreateTask: Interlocked.Increment(_lastTaskId).ToString().
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {};
        }
        const std::string task_id = std::to_string(++platform->last_task_id);
        request->set_task_id(task_id);
        platform->pending_invokes.emplace(task_id, request);
        return task_id;
    }

    void hybrid_web_view_handler::message_received(std::string_view raw_message)
    {
        // HybridWebViewHandler.MessageReceived → HybridWebViewHelper.ProcessRawMessage.
        const maui::core::hybrid_parsed_message parsed = maui::core::parse_hybrid_message(raw_message);
        auto* platform = typed_platform_view();
        switch (parsed.type)
        {
            case maui::core::hybrid_message_type::raw_message: {
                if (platform != nullptr)
                {
                    platform->received_raw_messages.push_back(parsed.content_or_task_id);
                }
                if (auto* view = virtual_view())
                {
                    // C#: virtualView?.RawMessageReceived(messageContent).
                    view->raw_message_received(parsed.content_or_task_id);
                }
                break;
            }
            case maui::core::hybrid_message_type::invoke_completed:
            case maui::core::hybrid_message_type::invoke_failed: {
                if (platform == nullptr)
                {
                    break;
                }
                const auto entry = platform->pending_invokes.find(parsed.content_or_task_id);
                if (entry == platform->pending_invokes.end())
                {
                    break; // HybridWebViewTaskManager.SetTask*: unknown id is silently dropped.
                }
                const std::shared_ptr<maui::core::invoke_java_script_request> request = entry->second;
                platform->pending_invokes.erase(entry); // TryRemove before completing.
                if (parsed.type == maui::core::hybrid_message_type::invoke_completed)
                {
                    // HybridWebViewHelper.ProcessInvokeJavaScriptAsync: a null/undefined/empty result is a
                    // null return; otherwise deliver the raw json (the caller deserializes).
                    if (parsed.invoke_result.empty() || parsed.invoke_result == "null" ||
                        parsed.invoke_result == "undefined")
                    {
                        request->complete(std::nullopt);
                    }
                    else
                    {
                        request->complete(parsed.invoke_result);
                    }
                }
                else
                {
                    request->fail(parsed.invoke_result.empty() ? std::nullopt
                                                               : std::optional<std::string>(parsed.invoke_result));
                }
                break;
            }
            case maui::core::hybrid_message_type::invalid:
                // C# throws ArgumentException on an unrecognized/malformed message; the port drops it so a
                // stray page message can't fault the seam.
                break;
        }
    }
} // namespace maui::controls
