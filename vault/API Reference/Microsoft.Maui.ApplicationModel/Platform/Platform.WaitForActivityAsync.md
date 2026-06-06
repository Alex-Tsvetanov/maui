---
title: "Platform.WaitForActivityAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.WaitForActivityAsync"
declaring_type: "Platform"
member_kind: method
---

# Platform.WaitForActivityAsync

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Waits for a `Activity` to be created or resumed.

## Signature

```csharp
System.Threading.Tasks.Task<Android.App.Activity!>! static WaitForActivityAsync(System.Threading.CancellationToken cancelToken = default(System.Threading.CancellationToken))
```

## Returns

The application's current `Activity` or the `Activity` that has been created or resumed.

## Parameters

| Parameter | Description |
|---|---|
| `cancelToken` | A token that can be used for cancelling the operation. |

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
