---
title: "ViewExtensions (Controls).RotateTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ViewExtensions.RotateTo"
declaring_type: "ViewExtensions (Controls)"
member_kind: method
---

# ViewExtensions (Controls).RotateTo

> [!abstract] Method of [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Returns a task that rotates the `VisualElement` that is specified by `view` that is described by the `rotation`, `length`, and `easing` parameters.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static RotateTo(this Microsoft.Maui.Controls.VisualElement! view, double rotation, uint length = 250, Microsoft.Maui.Easing? easing = null)
```

## Returns

A `Task` containing a `bool` value which indicates whether the animation was canceled. `true` indicates that the animation was canceled. `false` indicates that the animation ran to completion.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view on which this method operates. |
| `rotation` | The final rotation value. |
| `length` | The time, in milliseconds, over which to animate the transition. The default is 250. |
| `easing` | The easing function to use for the animation. |

## See also

- Declaring type: [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
