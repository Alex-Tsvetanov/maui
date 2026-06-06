---
title: "Layout (Compatibility).LayoutChildIntoBoundingRegion"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout.LayoutChildIntoBoundingRegion"
declaring_type: "Layout (Compatibility)"
member_kind: method
---

# Layout (Compatibility).LayoutChildIntoBoundingRegion

> [!abstract] Method of [[Layout (Compatibility)|Layout (Compatibility)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

Positions a child element into a bounding region while respecting the child elements `HorizontalOptions` and `VerticalOptions`.

## Signature

```csharp
void static LayoutChildIntoBoundingRegion(Microsoft.Maui.Controls.VisualElement child, Microsoft.Maui.Graphics.Rect region)
```

## Parameters

| Parameter | Description |
|---|---|
| `child` | The child element to be positioned. |
| `region` | The bounding region in which the child should be positioned. |

## Remarks

This method is called in the layout cycle after the general regions for each child have been calculated. This method will handle positioning the element relative to the bounding region given if the bounding region given is larger than the child's desired size.

## See also

- Declaring type: [[Layout (Compatibility)|Layout (Compatibility)]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
