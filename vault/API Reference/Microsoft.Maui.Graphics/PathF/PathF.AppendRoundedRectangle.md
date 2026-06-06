---
title: "PathF.AppendRoundedRectangle"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.AppendRoundedRectangle"
declaring_type: "PathF"
member_kind: method
---

# PathF.AppendRoundedRectangle

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Appends a rounded rectangle using the specified rectangle bounds and uniform corner radius.

## Signatures

```csharp
void AppendRoundedRectangle(float x, float y, float w, float h, float cornerRadius, bool includeLast = false)
void AppendRoundedRectangle(float x, float y, float w, float h, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius, bool includeLast = false)
void AppendRoundedRectangle(Microsoft.Maui.Graphics.RectF rect, float cornerRadius, bool includeLast = false)
void AppendRoundedRectangle(Microsoft.Maui.Graphics.RectF rect, float topLeftCornerRadius, float topRightCornerRadius, float bottomLeftCornerRadius, float bottomRightCornerRadius, bool includeLast = false)
void AppendRoundedRectangle(Microsoft.Maui.Graphics.RectF rect, float xCornerRadius, float yCornerRadius)
```

## Parameters

| Parameter | Description |
|---|---|
| `rect` | The rectangle bounds. |
| `cornerRadius` | Corner radius (clamped to half width/height). |
| `includeLast` | Include a duplicate final line before closing. |

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
