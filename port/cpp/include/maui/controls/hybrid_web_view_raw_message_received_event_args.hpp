#pragma once
// maui::controls::hybrid_web_view_raw_message_received_event_args
//   <=  Microsoft.Maui.Controls.HybridWebViewRawMessageReceivedEventArgs
//
// The argument of hybrid_web_view::raw_message_received. Ported from
// src/Controls/src/Core/HybridWebView/HybridWebViewRawMessageReceivedEventArgs.cs — a single read-only
// Message string. C#'s `string?` (nullable) is modeled as a std::optional<std::string>: every raw
// message that reaches the control through the protocol is non-null, but the ctor mirrors the nullable
// signature so the args type stays faithful.

#include <optional>
#include <string>

namespace maui::controls
{
    struct hybrid_web_view_raw_message_received_event_args
    {
        // C# HybridWebViewRawMessageReceivedEventArgs.Message — the raw message content (nullopt models
        // C#'s null).
        std::optional<std::string> message;
    };
} // namespace maui::controls
