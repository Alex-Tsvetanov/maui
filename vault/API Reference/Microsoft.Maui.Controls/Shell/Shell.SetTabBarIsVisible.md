---
title: "Shell.SetTabBarIsVisible"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Shell.SetTabBarIsVisible"
declaring_type: "Shell"
member_kind: method
---

# Shell.SetTabBarIsVisible

> [!abstract] Method of [[Shell|Shell]]
> Namespace: `Microsoft.Maui.Controls`

Sets the tabs visibility when the given `obj` is active.

## Signature

```csharp
void static SetTabBarIsVisible(Microsoft.Maui.Controls.BindableObject obj, bool value)
```

## Parameters

| Parameter | Description |
|---|---|
| `obj` | The object that modifies the tabs visibility. |
| `value` | `true` to set the tab bar as visible; otherwise, `false`. |

## Remarks

The tab bar and tabs are visible in Shell applications by default. However, the tab bar can be hidden by setting the Shell.TabBarIsVisible attached property to false. While this property can be set on a subclassed Shell object, it's typically set on any ShellContent or ContentPage objects that want to make the tab bar invisible.

## See also

- Declaring type: [[Shell|Shell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
