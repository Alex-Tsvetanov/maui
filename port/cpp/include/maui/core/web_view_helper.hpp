#pragma once
// maui::core::escape_js_string  <=  Microsoft.Maui.Handlers.WebViewHelper.EscapeJsString
//
// Escapes backslashes, single quotes, and line terminators in a JavaScript string for use inside the
// single-quoted eval('…') literal WebView.EvaluateJavaScriptAsync builds. Ported from
// src/Core/src/Handlers/WebView/WebViewHelper.cs. The U+2028/U+2029 line separators are matched as their
// UTF-8 byte sequences (the port's strings are UTF-8; C# scans UTF-16 chars).

#include <string>
#include <string_view>

namespace maui::core
{
    [[nodiscard]] std::string escape_js_string(std::string_view js);
} // namespace maui::core
