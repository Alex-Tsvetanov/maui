---
title: "Device.InvokeOnMainThreadAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Device.InvokeOnMainThreadAsync"
declaring_type: "Device"
member_kind: method
---

# Device.InvokeOnMainThreadAsync

> [!abstract] Method of [[Device|Device]]
> Namespace: `Microsoft.Maui.Controls`

Invokes an action on the main thread asynchronously.

## Signatures

```csharp
System.Threading.Tasks.Task static InvokeOnMainThreadAsync(System.Action action)
System.Threading.Tasks.Task static InvokeOnMainThreadAsync(System.Func<System.Threading.Tasks.Task> funcTask)
```

## Parameters

| Parameter | Description |
|---|---|
| `action` | The action to invoke. |

## Returns

A task representing the async operation.

## See also

- Declaring type: [[Device|Device]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
