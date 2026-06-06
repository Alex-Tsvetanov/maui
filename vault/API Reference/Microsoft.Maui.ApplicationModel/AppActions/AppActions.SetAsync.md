---
title: "AppActions.SetAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppActions.SetAsync"
declaring_type: "AppActions"
member_kind: method
---

# AppActions.SetAsync

> [!abstract] Method of [[AppActions|AppActions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Sets the app actions that will be available for this app.

## Signatures

```csharp
System.Threading.Tasks.Task! static SetAsync(params Microsoft.Maui.ApplicationModel.AppAction![]! actions)
System.Threading.Tasks.Task! static SetAsync(System.Collections.Generic.IEnumerable<Microsoft.Maui.ApplicationModel.AppAction!>! actions)
```

## Parameters

| Parameter | Description |
|---|---|
| `actions` | `AppAction` objects that will be set for this app. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[AppActions|AppActions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
