---
title: "Animation (Controls).WithConcurrent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Animation.WithConcurrent"
declaring_type: "Animation (Controls)"
member_kind: method
---

# Animation (Controls).WithConcurrent

> [!abstract] Method of [[Animation (Controls)|Animation (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Adds `animation` to the children of this `Animation` object and sets the start and end times of `animation` to `beginAt` and `finishAt`, respectively.

## Signatures

```csharp
Microsoft.Maui.Controls.Animation WithConcurrent(Microsoft.Maui.Controls.Animation animation, double beginAt = 0, double finishAt = 1)
Microsoft.Maui.Controls.Animation WithConcurrent(System.Action<double> callback, double start = 0, double end = 1, Microsoft.Maui.Easing easing = null, double beginAt = 0, double finishAt = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `animation` | The animation to add. |
| `beginAt` | The fraction into this animation at which the added child animation will begin animating. |
| `finishAt` | The fraction into this animation at which the added child animation will stop animating. |

## See also

- Declaring type: [[Animation (Controls)|Animation (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
