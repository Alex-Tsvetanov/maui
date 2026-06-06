---
title: "IAppActions.SetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IAppActions.SetAsync"
declaring_type: "IAppActions"
member_kind: method
---

# IAppActions.SetAsync

> [!abstract] Method of [[IAppActions|IAppActions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Sets the app actions that will be available for this app.

## Signature

```csharp
System.Threading.Tasks.Task! SetAsync(System.Collections.Generic.IEnumerable<Microsoft.Maui.ApplicationModel.AppAction!>! actions)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `actions` | A collection of `AppAction` that is to be set for this app. |

## See also

- Declaring type: [[IAppActions|IAppActions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
