---
title: "MainThread.InvokeOnMainThreadAsync<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.MainThread.InvokeOnMainThreadAsync<T>"
declaring_type: "MainThread"
member_kind: method
---

# MainThread.InvokeOnMainThreadAsync<T>

> [!abstract] Method of [[MainThread|MainThread]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Invokes an action on the main thread of the application asynchronously.

## Signatures

```csharp
System.Threading.Tasks.Task<T> static InvokeOnMainThreadAsync<T>(System.Func<System.Threading.Tasks.Task<T>> funcTask)
System.Threading.Tasks.Task<T> static InvokeOnMainThreadAsync<T>(System.Func<T> func)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `action` | The action to invoke on the main thread. |

## See also

- Declaring type: [[MainThread|MainThread]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
