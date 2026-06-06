---
title: "AnimationExtensions.Animate<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AnimationExtensions.Animate<T>"
declaring_type: "AnimationExtensions"
member_kind: method
---

# AnimationExtensions.Animate<T>

> [!abstract] Method of [[AnimationExtensions|AnimationExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Runs `animation` on `self`.

## Signature

```csharp
void static Animate<T>(this Microsoft.Maui.Controls.IAnimatable self, string name, System.Func<double, T> transform, System.Action<T> callback, uint rate = 16, uint length = 250, Microsoft.Maui.Easing easing = null, System.Action<T, bool> finished = null, System.Func<bool> repeat = null, Microsoft.Maui.Animations.IAnimationManager animationManager = null)
```

## See also

- Declaring type: [[AnimationExtensions|AnimationExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
