---
title: "ICanvas.Rotate"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.ICanvas.Rotate"
declaring_type: "ICanvas"
member_kind: method
---

# ICanvas.Rotate

> [!abstract] Method of [[ICanvas|ICanvas]]
> Namespace: `Microsoft.Maui.Graphics`

Rotates a graphical object around a point.

## Signatures

```csharp
void Rotate(float degrees, float x, float y)
void Rotate(float degrees)
```

## Parameters

| Parameter | Description |
|---|---|
| `degrees` | Rotation angle. |
| `x` | x coordinate of the rotation point. |
| `y` | y coordinate of the rotation point. |

## Remarks

Rotation is clockwise for increasing angles. Negative angles and angles greater than 360 are allowed.

## See also

- Declaring type: [[ICanvas|ICanvas]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
