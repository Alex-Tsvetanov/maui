---
title: "WebView (Controls).EvaluateJavaScriptAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.WebView.EvaluateJavaScriptAsync"
declaring_type: "WebView (Controls)"
member_kind: method
---

# WebView (Controls).EvaluateJavaScriptAsync

> [!abstract] Method of [[WebView (Controls)|WebView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

On platforms that support JavaScript evaluation, evaluates `script`.

## Signature

```csharp
System.Threading.Tasks.Task<string> EvaluateJavaScriptAsync(string script)
```

## Remarks

Native JavaScript evaluation is supported neither on Tizen nor GTK (Linux).

## Returns

A task that contains the result of the evaluation as a string.

## Parameters

| Parameter | Description |
|---|---|
| `script` | The script to evaluate. |

## See also

- Declaring type: [[WebView (Controls)|WebView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
