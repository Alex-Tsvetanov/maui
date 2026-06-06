---
title: "AppActionsExtensions.OnNewIntent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppActionsExtensions.OnNewIntent"
declaring_type: "AppActionsExtensions"
member_kind: method
---

# AppActionsExtensions.OnNewIntent

> [!abstract] Method of [[AppActionsExtensions|AppActionsExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
void static OnNewIntent(this Microsoft.Maui.ApplicationModel.IAppActions! appActions, Android.Content.Intent? intent)
```

## Parameters

| Parameter | Description |
|---|---|
| `appActions` | Instance of the `IAppActions` object this event is invoked on. |
| `intent` | The provided `Intent` to launch this app with. |

## See also

- Declaring type: [[AppActionsExtensions|AppActionsExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
