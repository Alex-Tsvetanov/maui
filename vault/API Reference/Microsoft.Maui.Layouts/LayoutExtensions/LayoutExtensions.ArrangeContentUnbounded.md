---
title: "LayoutExtensions.ArrangeContentUnbounded"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Layouts
aliases:
  - "Microsoft.Maui.Layouts.LayoutExtensions.ArrangeContentUnbounded"
declaring_type: "LayoutExtensions"
member_kind: method
---

# LayoutExtensions.ArrangeContentUnbounded

> [!abstract] Method of [[LayoutExtensions|LayoutExtensions]]
> Namespace: `Microsoft.Maui.Layouts`

Arranges content which can exceed the bounds of the IContentView.

## Signature

```csharp
Microsoft.Maui.Graphics.Size static ArrangeContentUnbounded(this Microsoft.Maui.IContentView! contentView, Microsoft.Maui.Graphics.Rect bounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `contentView` |  |
| `bounds` |  |

## Returns

The Size of the arranged content

## Remarks

Useful for arranging content where the IContentView provides a viewport to a portion of the content (e.g, the content of an IScrollView).

## See also

- Declaring type: [[LayoutExtensions|LayoutExtensions]]
- [[_Microsoft.Maui.Layouts|Microsoft.Maui.Layouts namespace]]
