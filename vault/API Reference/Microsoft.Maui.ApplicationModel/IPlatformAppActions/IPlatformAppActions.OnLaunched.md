---
title: "IPlatformAppActions.OnLaunched"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IPlatformAppActions.OnLaunched"
declaring_type: "IPlatformAppActions"
member_kind: method
---

# IPlatformAppActions.OnLaunched

> [!abstract] Method of [[IPlatformAppActions|IPlatformAppActions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
System.Threading.Tasks.Task! OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs! e)
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `e` | Event arguments containing information about the launch of the application. |

## See also

- Declaring type: [[IPlatformAppActions|IPlatformAppActions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
