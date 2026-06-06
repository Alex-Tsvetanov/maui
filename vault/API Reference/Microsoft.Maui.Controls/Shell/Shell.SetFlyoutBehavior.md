---
title: "Shell.SetFlyoutBehavior"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Shell.SetFlyoutBehavior"
declaring_type: "Shell"
member_kind: method
---

# Shell.SetFlyoutBehavior

> [!abstract] Method of [[Shell|Shell]]
> Namespace: `Microsoft.Maui.Controls`

Sets the behavior used to open the flyout when the given `obj` is presented.

## Signature

```csharp
void static SetFlyoutBehavior(Microsoft.Maui.Controls.BindableObject obj, Microsoft.Maui.FlyoutBehavior value)
```

## Parameters

| Parameter | Description |
|---|---|
| `obj` | The object that modifies the Shell behavior used to open the flyout. |
| `value` | The behavior used to open the flyout. |

## Remarks

The flyout can be accessed through the hamburger icon or by swiping from the side of the screen. However, this behavior can be changed by setting the attached property.

## See also

- Declaring type: [[Shell|Shell]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
