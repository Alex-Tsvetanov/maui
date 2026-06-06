---
title: "PathF.AppendRectangle"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.AppendRectangle"
declaring_type: "PathF"
member_kind: method
---

# PathF.AppendRectangle

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Appends a rectangle path using the specified rectangle bounds.

## Signatures

```csharp
void AppendRectangle(float x, float y, float w, float h, bool includeLast = false)
void AppendRectangle(Microsoft.Maui.Graphics.RectF rect, bool includeLast = false)
```

## Parameters

| Parameter | Description |
|---|---|
| `rect` | The rectangle bounds. |
| `includeLast` | Include a final duplicate line to the first point before closing. |

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
