---
title: "AnimationExtensions.Interpolate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AnimationExtensions.Interpolate"
declaring_type: "AnimationExtensions"
member_kind: method
---

# AnimationExtensions.Interpolate

> [!abstract] Method of [[AnimationExtensions|AnimationExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Returns a function that performs a linear interpolation between `start` and `end`.

## Signature

```csharp
System.Func<double, double> static Interpolate(double start, double end = 1, double reverseVal = 0, bool reverse = false)
```

## Remarks

If `reverse` is `true`, then the interpolation happens between `start` and `reverseVal`.

## Returns

A function that performs a linear interpolation between `start` and `end`. Application developers can pass values between 0.0f and 1.0f to this function in order to receive a value that is offset from `start` or `end`, depending on the value of `reverse`, by the passed value times the distance between `start` and `end`.

## Parameters

| Parameter | Description |
|---|---|
| `start` | The fraction into the current animation at which to start the animation. |
| `end` | The fraction into the current animation at which to stop the animation. |
| `reverseVal` | The inverse scale factor to use if `reverse` is `true`. |
| `reverse` | Whether to use the inverse scale factor in `reverseVal` to deinterpolate. |

## See also

- Declaring type: [[AnimationExtensions|AnimationExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
