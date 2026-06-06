---
title: "IHybridWebView.InvokeJavaScriptAsync<TReturnType>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IHybridWebView.InvokeJavaScriptAsync<TReturnType>"
declaring_type: "IHybridWebView"
member_kind: method
---

# IHybridWebView.InvokeJavaScriptAsync<TReturnType>

> [!abstract] Method of [[IHybridWebView|IHybridWebView]]
> Namespace: `Microsoft.Maui`

Invokes a JavaScript method named `methodName` and optionally passes in the parameter values specified by `paramValues` by JSON-encoding each one.

## Signature

```csharp
System.Threading.Tasks.Task<TReturnType?>! InvokeJavaScriptAsync<TReturnType>(string! methodName, System.Text.Json.Serialization.Metadata.JsonTypeInfo<TReturnType>! returnTypeJsonTypeInfo, object?[]? paramValues = null, System.Text.Json.Serialization.Metadata.JsonTypeInfo?[]? paramJsonTypeInfos = null)
```

## Returns

An object of type `TReturnType` containing the return value of the called method.

## Parameters

| Parameter | Description |
|---|---|
| `methodName` | The name of the JavaScript method to invoke. |
| `returnTypeJsonTypeInfo` | Metadata about deserializing the type of the return value specified by `TReturnType`. |
| `paramValues` | Optional array of objects to be passed to the JavaScript method by JSON-encoding each one. |
| `paramJsonTypeInfos` | Optional array of metadata about serializing the types of the parameters specified by `paramValues`. |

## See also

- Declaring type: [[IHybridWebView|IHybridWebView]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
