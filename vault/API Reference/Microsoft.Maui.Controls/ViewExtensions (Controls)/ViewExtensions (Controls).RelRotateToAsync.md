---
title: "ViewExtensions (Controls).RelRotateToAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ViewExtensions.RelRotateToAsync"
declaring_type: "ViewExtensions (Controls)"
member_kind: method
---

# ViewExtensions (Controls).RelRotateToAsync

> [!abstract] Method of [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Rotates the `VisualElement` that is specified by `view` from its current rotation by `drotation`.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static RelRotateToAsync(this Microsoft.Maui.Controls.VisualElement! view, double drotation, uint length = 250, Microsoft.Maui.Easing? easing = null)
```

## Returns

A `Task` containing a `bool` value which indicates whether the animation was canceled. `true` indicates that the animation was canceled. `false` indicates that the animation ran to completion.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view on which this method operates. |
| `drotation` | The relative rotation. |
| `length` | The time, in milliseconds, over which to animate the transition. The default is 250. |
| `easing` | The easing function to use for the animation. |

## See also

- Declaring type: [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
