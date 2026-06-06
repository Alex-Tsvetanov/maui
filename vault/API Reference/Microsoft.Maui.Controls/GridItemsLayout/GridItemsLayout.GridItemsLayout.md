---
title: "GridItemsLayout.GridItemsLayout"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GridItemsLayout.GridItemsLayout"
declaring_type: "GridItemsLayout"
member_kind: constructor
---

# GridItemsLayout.GridItemsLayout

> [!abstract] Constructor of [[GridItemsLayout|GridItemsLayout]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the number of columns (for vertical orientation) or rows (for horizontal orientation) in the grid.

## Signatures

```csharp
void GridItemsLayout(int span, Microsoft.Maui.Controls.ItemsLayoutOrientation orientation)
void GridItemsLayout(Microsoft.Maui.Controls.ItemsLayoutOrientation orientation)
```

## Parameters

| Parameter | Description |
|---|---|
| `orientation` | The scroll orientation of the grid. |

## Remarks

For vertical scrolling grids, `Span` determines the number of columns. For horizontal scrolling grids, `Span` determines the number of rows.

## See also

- Declaring type: [[GridItemsLayout|GridItemsLayout]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
