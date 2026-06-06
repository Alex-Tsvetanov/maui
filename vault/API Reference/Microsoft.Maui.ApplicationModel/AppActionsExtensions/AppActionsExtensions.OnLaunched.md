---
title: "AppActionsExtensions.OnLaunched"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppActionsExtensions.OnLaunched"
declaring_type: "AppActionsExtensions"
member_kind: method
---

# AppActionsExtensions.OnLaunched

> [!abstract] Method of [[AppActionsExtensions|AppActionsExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
System.Threading.Tasks.Task! static OnLaunched(this Microsoft.Maui.ApplicationModel.IAppActions! appActions, Microsoft.UI.Xaml.LaunchActivatedEventArgs! e)
```

## Parameters

| Parameter | Description |
|---|---|
| `appActions` | Instance of the `IAppActions` object this event is invoked on. |
| `e` | Event arguments containing information about the launch of the application. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[AppActionsExtensions|AppActionsExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
