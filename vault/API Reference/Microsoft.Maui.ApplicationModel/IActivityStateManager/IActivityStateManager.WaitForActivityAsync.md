---
title: "IActivityStateManager.WaitForActivityAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IActivityStateManager.WaitForActivityAsync"
declaring_type: "IActivityStateManager"
member_kind: method
---

# IActivityStateManager.WaitForActivityAsync

> [!abstract] Method of [[IActivityStateManager|IActivityStateManager]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Waits for a `Activity` to be created or resumed.

## Signature

```csharp
System.Threading.Tasks.Task<Android.App.Activity!>! WaitForActivityAsync(System.Threading.CancellationToken cancelToken = default(System.Threading.CancellationToken))
```

## Returns

The application's current `Activity` or the `Activity` that has been created or resumed.

## Parameters

| Parameter | Description |
|---|---|
| `cancelToken` | A token that can be used for cancelling the operation. |

## See also

- Declaring type: [[IActivityStateManager|IActivityStateManager]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
