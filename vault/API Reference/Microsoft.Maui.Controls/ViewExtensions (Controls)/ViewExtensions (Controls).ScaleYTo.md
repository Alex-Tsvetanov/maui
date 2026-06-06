---
title: "ViewExtensions (Controls).ScaleYTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ViewExtensions.ScaleYTo"
declaring_type: "ViewExtensions (Controls)"
member_kind: method
---

# ViewExtensions (Controls).ScaleYTo

> [!abstract] Method of [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Returns a task that scales the Y axis of the the `VisualElement` that is specified by `view` to the absolute scale factor `scale`.

## Signature

```csharp
System.Threading.Tasks.Task<bool>! static ScaleYTo(this Microsoft.Maui.Controls.VisualElement! view, double scale, uint length = 250, Microsoft.Maui.Easing? easing = null)
```

## Returns

A `Task` containing a `bool` value which indicates whether the animation was canceled. `true` indicates that the animation was canceled. `false` indicates that the animation ran to completion.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view on which this method operates. |
| `scale` | The final absolute scale. |
| `length` | The time, in milliseconds, over which to animate the transition. The default is 250. |
| `easing` | The easing function to use for the animation. |

## See also

- Declaring type: [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
