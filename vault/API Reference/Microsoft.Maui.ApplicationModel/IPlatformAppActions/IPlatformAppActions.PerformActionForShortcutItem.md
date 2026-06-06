---
title: "IPlatformAppActions.PerformActionForShortcutItem"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IPlatformAppActions.PerformActionForShortcutItem"
declaring_type: "IPlatformAppActions"
member_kind: method
---

# IPlatformAppActions.PerformActionForShortcutItem

> [!abstract] Method of [[IPlatformAppActions|IPlatformAppActions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

The lifecycle event that is triggered when this app is launched.

## Signature

```csharp
void PerformActionForShortcutItem(UIKit.UIApplication! application, UIKit.UIApplicationShortcutItem! shortcutItem, UIKit.UIOperationHandler! completionHandler)
```

## Parameters

| Parameter | Description |
|---|---|
| `application` | The `UIApplication` instance this action is performed for. |
| `shortcutItem` | The shortcut item that was chosen from the app icon. |
| `completionHandler` | The completion handler that is triggered when this action has completed. |

## See also

- Declaring type: [[IPlatformAppActions|IPlatformAppActions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
