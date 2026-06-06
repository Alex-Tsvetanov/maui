---
title: "WebView (Controls).Eval"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebView.Eval"
declaring_type: "WebView (Controls)"
member_kind: method
---

# WebView (Controls).Eval

> [!abstract] Method of [[WebView (Controls)|WebView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets a value that indicates whether the user can navigate to previous pages.

## Signature

```csharp
void Eval(string script)
```

## Parameters

| Parameter | Description |
|---|---|
| `script` | A script to evaluate. |

## Remarks

The default value is the default User Agent of the underlying platform browser, or `null` if it cannot be determined. If the parameter is `null` the User Agent will not be updated and the current User Agent will remain.

## See also

- Declaring type: [[WebView (Controls)|WebView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
