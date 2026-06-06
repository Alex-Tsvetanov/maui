---
title: "AnimationLerpingExtensions.GenericLerp<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Animations
aliases:
  - "Microsoft.Maui.Animations.AnimationLerpingExtensions.GenericLerp<T>"
declaring_type: "AnimationLerpingExtensions"
member_kind: method
---

# AnimationLerpingExtensions.GenericLerp<T>

> [!abstract] Method of [[AnimationLerpingExtensions|AnimationLerpingExtensions]]
> Namespace: `Microsoft.Maui.Animations`

Linearly interpolates between two values of an arbitrary type based on the given progress, using a toggle threshold for non-interpolatable types.

## Signature

```csharp
T static GenericLerp<T>(this T start, T end, double progress, double toggleThreshold = 0.5)
```

## See also

- Declaring type: [[AnimationLerpingExtensions|AnimationLerpingExtensions]]
- [[_Microsoft.Maui.Animations|Microsoft.Maui.Animations namespace]]
