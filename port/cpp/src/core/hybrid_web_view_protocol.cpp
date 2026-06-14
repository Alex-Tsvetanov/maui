// hybrid_web_view_protocol — the cross-platform hybrid-web-view message protocol (the host side of
// HybridWebView.js + HybridWebViewHelper.ProcessRawMessage). Pure string work, no platform types.

#include "maui/core/hybrid_web_view_protocol.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace maui::core
{
    namespace
    {
        constexpr std::string_view raw_message_type = "__RawMessage";
        constexpr std::string_view invoke_completed_type = "__InvokeJavaScriptCompleted";
        constexpr std::string_view invoke_failed_type = "__InvokeJavaScriptFailed";
    } // namespace

    std::string json_quote(std::string_view value)
    {
        // System.Text.Json's default string serialization: wrap in quotes and escape the control set.
        // Backslash / double-quote / the C0 control chars (\b \f \n \r \t and the generic \uXXXX form)
        // are the escapes the HybridWebView protocol relies on; anything else passes through as UTF-8.
        std::string result;
        result.reserve(value.size() + 2);
        result.push_back('"');
        for (const char raw : value)
        {
            const auto byte = static_cast<unsigned char>(raw);
            switch (raw)
            {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\b':
                    result += "\\b";
                    break;
                case '\f':
                    result += "\\f";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    if (byte < 0x20U)
                    {
                        static constexpr std::array<char, 16> hex_digits = {'0', '1', '2', '3', '4', '5', '6', '7',
                                                                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                        result += "\\u00";
                        result.push_back(hex_digits.at(byte >> 4U));
                        result.push_back(hex_digits.at(byte & 0x0FU));
                    }
                    else
                    {
                        result.push_back(raw);
                    }
                    break;
            }
        }
        result.push_back('"');
        return result;
    }

    hybrid_parsed_message parse_hybrid_message(std::string_view raw_message)
    {
        hybrid_parsed_message parsed;
        if (raw_message.empty())
        {
            return parsed; // C# throws on a null/empty message; the port reports invalid.
        }
        const std::size_t pipe = raw_message.find('|');
        if (pipe == std::string_view::npos)
        {
            return parsed; // C# requires a pipe; invalid otherwise.
        }
        const std::string_view message_type = raw_message.substr(0, pipe);
        const std::string_view content = raw_message.substr(pipe + 1);

        if (message_type == raw_message_type)
        {
            parsed.type = hybrid_message_type::raw_message;
            parsed.content_or_task_id = std::string(content);
            return parsed;
        }
        if (message_type == invoke_completed_type || message_type == invoke_failed_type)
        {
            // The invoke content is itself "<taskId>|<result>".
            const std::size_t inner_pipe = content.find('|');
            if (inner_pipe == std::string_view::npos)
            {
                return parsed; // C# throws when the invoke content lacks a pipe; invalid here.
            }
            parsed.type = message_type == invoke_completed_type ? hybrid_message_type::invoke_completed
                                                                : hybrid_message_type::invoke_failed;
            parsed.content_or_task_id = std::string(content.substr(0, inner_pipe));
            parsed.invoke_result = std::string(content.substr(inner_pipe + 1));
            return parsed;
        }
        // An unrecognized message type (C# throws ArgumentException) — report invalid.
        return parsed;
    }

    std::string build_send_raw_message_script(std::string_view message)
    {
        // MauiHybridWebView.SendRawMessage:
        // `window.external.receiveMessage(<JsonSerializer.Serialize(message)>)`.
        std::string script = "window.external.receiveMessage(";
        script += json_quote(message);
        script.push_back(')');
        return script;
    }

    std::string build_invoke_java_script_script(std::string_view task_id, std::string_view method_name,
                                                const std::vector<std::string>& param_values)
    {
        // HybridWebViewHelper.ProcessInvokeJavaScriptAsync:
        // `window.HybridWebView.__InvokeJavaScript(<taskId>, <methodName>, [<params...>])`.
        std::string script = "window.HybridWebView.__InvokeJavaScript(";
        script += task_id;
        script += ", ";
        script += method_name;
        script += ", [";
        for (std::size_t i = 0; i < param_values.size(); ++i)
        {
            if (i != 0)
            {
                script += ", ";
            }
            script += param_values[i];
        }
        script += "])";
        return script;
    }
} // namespace maui::core
