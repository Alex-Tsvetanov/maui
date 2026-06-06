---
title: "AppActionsExtensions.PerformActionForShortcutItem"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppActionsExtensions.PerformActionForShortcutItem"
declaring_type: "AppActionsExtensions"
member_kind: method
---

# AppActionsExtensions.PerformActionForShortcutItem

> [!abstract] Method of [[AppActionsExtensions|AppActionsExtensions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
void static PerformActionForShortcutItem(this Microsoft.Maui.ApplicationModel.IAppActions! appActions, UIKit.UIApplication! application, UIKit.UIApplicationShortcutItem! shortcutItem, UIKit.UIOperationHandler! completionHandler)
```

## Parameters

| Parameter | Description |
|---|---|
| `appActions` | Instance of the `IAppActions` object this event is invoked on. |
| `application` | The `UIApplication` instance this action is performed for. |
| `shortcutItem` | The shortcut item that was chosen from the app icon. |
| `completionHandler` | The completion handler that is triggered when this action has completed. |

## See also

- Declaring type: [[AppActionsExtensions|AppActionsExtensions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
