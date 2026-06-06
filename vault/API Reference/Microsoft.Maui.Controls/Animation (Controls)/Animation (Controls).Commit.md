---
title: "Animation (Controls).Commit"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Animation.Commit"
declaring_type: "Animation (Controls)"
member_kind: method
---

# Animation (Controls).Commit

> [!abstract] Method of [[Animation (Controls)|Animation (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Runs the `owner` animation with the supplied parameters.

## Signature

```csharp
void Commit(Microsoft.Maui.Controls.IAnimatable owner, string name, uint rate = 16, uint length = 250, Microsoft.Maui.Easing easing = null, System.Action<double, bool> finished = null, System.Func<bool> repeat = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `owner` | The owning animation that will be animated. |
| `name` | The name, or handle, that is used to access and track the animation and its state. |
| `rate` | The time, in milliseconds, between frames. |
| `length` | The number of milliseconds over which to interpolate the animation. |
| `easing` | The easing function to use to transition in, out, or in and out of the animation. |
| `finished` | An action to call when the animation is finished. |
| `repeat` | A function that should return true if the animation should continue. |

## See also

- Declaring type: [[Animation (Controls)|Animation (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
