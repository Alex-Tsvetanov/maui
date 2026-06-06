---
title: "ViewExtensions (Controls).TranslateTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ViewExtensions.TranslateTo"
declaring_type: "ViewExtensions (Controls)"
member_kind: method
---

# ViewExtensions (Controls).TranslateTo

> [!abstract] Method of [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Animates an elements `TranslationX` and `TranslationY` properties from their current values to the new values. This ensures that the input layout is in the same position as the visual layout.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static TranslateTo(this Microsoft.Maui.Controls.VisualElement! view, double x, double y, uint length = 250, Microsoft.Maui.Easing? easing = null)
```

## Returns

A `Task` containing a `bool` value which indicates whether the animation was canceled. `true` indicates that the animation was canceled. `false` indicates that the animation ran to completion.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view on which this method operates. |
| `x` | The x component of the final translation vector. |
| `y` | The y component of the final translation vector. |
| `length` | The time, in milliseconds, over which to animate the transition. The default is 250. |
| `easing` | The easing function to use for the animation. |

## See also

- Declaring type: [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
