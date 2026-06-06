---
title: "WebView (Controls).UserAgent"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebView.UserAgent"
declaring_type: "WebView (Controls)"
member_kind: property
---

# WebView (Controls).UserAgent

> [!abstract] Property of [[WebView (Controls)|WebView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the user agent string that this `WebView` object uses.

## Signature

```csharp
string UserAgent { get; set; }
```

## Remarks

The default value is the default User Agent of the underlying platform browser, or `null` if it cannot be determined. If the parameter is `null` the User Agent will not be updated and the current User Agent will remain.

## See also

- Declaring type: [[WebView (Controls)|WebView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
