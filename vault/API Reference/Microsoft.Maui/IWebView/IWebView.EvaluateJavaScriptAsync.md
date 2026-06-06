---
title: "IWebView.EvaluateJavaScriptAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IWebView.EvaluateJavaScriptAsync"
declaring_type: "IWebView"
member_kind: method
---

# IWebView.EvaluateJavaScriptAsync

> [!abstract] Method of [[IWebView|IWebView]]
> Namespace: `Microsoft.Maui`

On platforms that support JavaScript evaluation, evaluates script.

## Signature

```csharp
System.Threading.Tasks.Task<string!>! EvaluateJavaScriptAsync(string! script)
```

## Returns

A task that contains the result of the evaluation as a string.

## Parameters

| Parameter | Description |
|---|---|
| `script` | The script to evaluate. |

## See also

- Declaring type: [[IWebView|IWebView]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
