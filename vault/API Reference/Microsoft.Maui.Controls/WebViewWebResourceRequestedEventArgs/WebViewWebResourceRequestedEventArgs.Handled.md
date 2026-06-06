---
title: "WebViewWebResourceRequestedEventArgs.Handled"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebViewWebResourceRequestedEventArgs.Handled"
declaring_type: "WebViewWebResourceRequestedEventArgs"
member_kind: property
---

# WebViewWebResourceRequestedEventArgs.Handled

> [!abstract] Property of [[WebViewWebResourceRequestedEventArgs|WebViewWebResourceRequestedEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether the request has been handled. If set to true, the web view will not process the request further and a response must be provided using the `SetResponse` or `SetResponse` method. If set to false, the web view will continue processing the request as normal.

## Signature

```csharp
bool Handled { get; set; }
```

## See also

- Declaring type: [[WebViewWebResourceRequestedEventArgs|WebViewWebResourceRequestedEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
