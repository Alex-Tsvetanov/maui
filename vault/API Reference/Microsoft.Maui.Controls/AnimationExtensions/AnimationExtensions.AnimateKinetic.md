---
title: "AnimationExtensions.AnimateKinetic"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AnimationExtensions.AnimateKinetic"
declaring_type: "AnimationExtensions"
member_kind: method
---

# AnimationExtensions.AnimateKinetic

> [!abstract] Method of [[AnimationExtensions|AnimationExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Sets the specified parameters and starts the kinetic animation.

## Signature

```csharp
void static AnimateKinetic(this Microsoft.Maui.Controls.IAnimatable self, string name, System.Func<double, double, bool> callback, double velocity, double drag, System.Action finished = null, Microsoft.Maui.Animations.IAnimationManager animationManager = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `self` | The object on which this method will be run. |
| `name` | An animation key that should be unique among its sibling and parent animations for the duration of the animation. |
| `callback` | An action that is called with successive animation values. |
| `velocity` | The amount that the animation progresses in each animation step. For example, a velocity of 1 progresses at the default speed. |
| `drag` | The amount that the progression speed is reduced per frame. Can be negative. |
| `finished` | An action to call when the animation is finished. |
| `animationManager` | The animation manager to use for this animation. If null, the default animation manager for the target object will be used. |

## See also

- Declaring type: [[AnimationExtensions|AnimationExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
