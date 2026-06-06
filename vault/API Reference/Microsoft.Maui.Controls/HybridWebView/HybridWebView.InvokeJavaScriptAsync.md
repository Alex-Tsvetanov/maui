---
title: "HybridWebView.InvokeJavaScriptAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.HybridWebView.InvokeJavaScriptAsync"
declaring_type: "HybridWebView"
member_kind: method
---

# HybridWebView.InvokeJavaScriptAsync

> [!abstract] Method of [[HybridWebView|HybridWebView]]
> Namespace: `Microsoft.Maui.Controls`

Invokes a JavaScript method named `methodName` and optionally passes in the parameter values specified by `paramValues` by JSON-encoding each one.

## Signature

```csharp
System.Threading.Tasks.Task! InvokeJavaScriptAsync(string! methodName, object?[]? paramValues = null, System.Text.Json.Serialization.Metadata.JsonTypeInfo?[]? paramJsonTypeInfos = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `methodName` | The name of the JavaScript method to invoke. |
| `paramValues` | Optional array of objects to be passed to the JavaScript method by JSON-encoding each one. |
| `paramJsonTypeInfos` | Optional array of metadata about serializing the types of the parameters specified by `paramValues`. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[HybridWebView|HybridWebView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
