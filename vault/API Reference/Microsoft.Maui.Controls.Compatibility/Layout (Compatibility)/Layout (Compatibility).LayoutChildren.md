---
title: "Layout (Compatibility).LayoutChildren"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout.LayoutChildren"
declaring_type: "Layout (Compatibility)"
member_kind: method
---

# Layout (Compatibility).LayoutChildren

> [!abstract] Method of [[Layout (Compatibility)|Layout (Compatibility)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

Positions and sizes the children of a layout.

## Signature

```csharp
void abstract LayoutChildren(double x, double y, double width, double height)
```

## Parameters

| Parameter | Description |
|---|---|
| `x` | A value representing the x coordinate of the child region bounding box. |
| `y` | A value representing the y coordinate of the child region bounding box. |
| `width` | A value representing the width of the child region bounding box. |
| `height` | A value representing the height of the child region bounding box. |

## Remarks

Implementors wishing to change the default behavior of a Layout should override this method. It is suggested to still call the base method and modify its calculated results.

## See also

- Declaring type: [[Layout (Compatibility)|Layout (Compatibility)]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
