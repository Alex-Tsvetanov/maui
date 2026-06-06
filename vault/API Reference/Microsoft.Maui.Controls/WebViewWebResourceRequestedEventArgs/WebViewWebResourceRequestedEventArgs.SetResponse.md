---
title: "WebViewWebResourceRequestedEventArgs.SetResponse"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs.SetResponse"
declaring_type: "WebViewWebResourceRequestedEventArgs"
member_kind: method
---

# WebViewWebResourceRequestedEventArgs.SetResponse

> [!abstract] Method of [[WebViewWebResourceRequestedEventArgs|WebViewWebResourceRequestedEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Sets the response for the web resource request. This method must be called if the `Handled` property is set to true.

## Signatures

```csharp
void SetResponse(int code, string! reason, string! contentType, System.IO.Stream? content)
void SetResponse(int code, string! reason, string! contentType, System.Threading.Tasks.Task<System.IO.Stream?>! contentTask)
void SetResponse(int code, string! reason, System.Collections.Generic.IReadOnlyDictionary<string!, string!>? headers, System.IO.Stream? content)
void SetResponse(int code, string! reason, System.Collections.Generic.IReadOnlyDictionary<string!, string!>? headers, System.Threading.Tasks.Task<System.IO.Stream?>! contentTask)
void SetResponse(int code, string! reason)
```

## Parameters

| Parameter | Description |
|---|---|
| `code` | The HTTP status code for the response. |
| `reason` | The reason phrase for the response. |
| `headers` | The headers to include in the response. |
| `content` | The content of the response as a stream. |

## See also

- Declaring type: [[WebViewWebResourceRequestedEventArgs|WebViewWebResourceRequestedEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
