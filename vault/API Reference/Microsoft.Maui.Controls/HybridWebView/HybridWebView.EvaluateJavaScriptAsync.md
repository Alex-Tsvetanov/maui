---
title: "HybridWebView.EvaluateJavaScriptAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.HybridWebView.EvaluateJavaScriptAsync"
declaring_type: "HybridWebView"
member_kind: method
---

# HybridWebView.EvaluateJavaScriptAsync

> [!abstract] Method of [[HybridWebView|HybridWebView]]
> Namespace: `Microsoft.Maui.Controls`

On platforms that support JavaScript evaluation, evaluates `script`.

## Signature

```csharp
System.Threading.Tasks.Task<string?>! EvaluateJavaScriptAsync(string! script)
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

- Declaring type: [[HybridWebView|HybridWebView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
