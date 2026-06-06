---
title: "AppActionsExtensions.OnResume"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppActionsExtensions.OnResume"
declaring_type: "AppActionsExtensions"
member_kind: method
---

# AppActionsExtensions.OnResume

> [!abstract] Method of [[AppActionsExtensions|AppActionsExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
void static OnResume(this Microsoft.Maui.ApplicationModel.IAppActions! appActions, Android.Content.Intent? intent)
```

## Parameters

| Parameter | Description |
|---|---|
| `appActions` | Instance of the `IAppActions` object this event is invoked on. |
| `intent` | The provided `Intent` to resume this app with. |

## See also

- Declaring type: [[AppActionsExtensions|AppActionsExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
