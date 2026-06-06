---
title: "PaintGradientStop.CompareTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PaintGradientStop.CompareTo"
declaring_type: "PaintGradientStop"
member_kind: method
---

# PaintGradientStop.CompareTo

> [!abstract] Method of [[PaintGradientStop|PaintGradientStop]]
> Namespace: `Microsoft.Maui.Graphics`

Gets or sets the color at this gradient stop.

## Signature

```csharp
int CompareTo(Microsoft.Maui.Graphics.PaintGradientStop obj)
```

## Parameters

| Parameter | Description |
|---|---|
| `obj` | The gradient stop to compare with this instance. |

## Returns

A value less than zero if this instance's offset is less than the other instance's offset; zero if they are equal; a value greater than zero if this instance's offset is greater.

## Remarks

Typically between 0.0 (start of gradient) and 1.0 (end of gradient).

## See also

- Declaring type: [[PaintGradientStop|PaintGradientStop]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
