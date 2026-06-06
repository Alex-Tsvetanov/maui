---
title: "Platform.PerformActionForShortcutItem"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Platform.PerformActionForShortcutItem"
declaring_type: "Platform"
member_kind: method
---

# Platform.PerformActionForShortcutItem

> [!abstract] Method of [[Platform|Platform]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Invokes the action that corresponds to the chosen `AppAction` by the user.

## Signature

```csharp
void static PerformActionForShortcutItem(UIKit.UIApplication! application, UIKit.UIApplicationShortcutItem! shortcutItem, UIKit.UIOperationHandler! completionHandler)
```

## Parameters

| Parameter | Description |
|---|---|
| `application` | The application this action is invoked on. |
| `shortcutItem` | The shortcut item that was chosen by the user. |
| `completionHandler` | The action that is invoked when the operation is completed. |

## See also

- Declaring type: [[Platform|Platform]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
