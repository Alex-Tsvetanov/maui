---
title: "VisualElement (Controls).Arrange"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Arrange"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).Arrange

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Positions child objects and determines a size for an element.

## Signature

```csharp
void Arrange(Microsoft.Maui.Graphics.Rect bounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The final size that the parent computes for the child in layout, provided as a `Rect` value. |

## Remarks

Parent objects that implement custom layout for their child elements should call this method from their layout override implementations to form a recursive layout update. Prior to .NET 9, this method simply called `Layout`. If you need to revert to the old behavior, just call `Layout`.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
