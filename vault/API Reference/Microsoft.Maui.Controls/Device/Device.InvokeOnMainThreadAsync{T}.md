---
title: "Device.InvokeOnMainThreadAsync<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Device.InvokeOnMainThreadAsync<T>"
declaring_type: "Device"
member_kind: method
---

# Device.InvokeOnMainThreadAsync<T>

> [!abstract] Method of [[Device|Device]]
> Namespace: `Microsoft.Maui.Controls`

Invokes an async function on the main thread and returns the result.

## Signatures

```csharp
System.Threading.Tasks.Task<T> static InvokeOnMainThreadAsync<T>(System.Func<System.Threading.Tasks.Task<T>> funcTask)
System.Threading.Tasks.Task<T> static InvokeOnMainThreadAsync<T>(System.Func<T> func)
```

## Returns

A task containing the result of the function.

## Parameters

| Parameter | Description |
|---|---|
| `func` | The function to invoke. |

## See also

- Declaring type: [[Device|Device]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
