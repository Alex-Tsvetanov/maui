---
title: "MainThread.InvokeOnMainThreadAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.MainThread.InvokeOnMainThreadAsync"
declaring_type: "MainThread"
member_kind: method
---

# MainThread.InvokeOnMainThreadAsync

> [!abstract] Method of [[MainThread|MainThread]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Invokes an action on the main thread of the application asynchronously.

## Signatures

```csharp
System.Threading.Tasks.Task static InvokeOnMainThreadAsync(System.Action action)
System.Threading.Tasks.Task static InvokeOnMainThreadAsync(System.Func<System.Threading.Tasks.Task> funcTask)
```

## Parameters

| Parameter | Description |
|---|---|
| `action` | The action to invoke on the main thread. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[MainThread|MainThread]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
