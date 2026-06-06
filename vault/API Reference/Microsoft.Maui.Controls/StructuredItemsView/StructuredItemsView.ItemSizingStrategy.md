---
title: "StructuredItemsView.ItemSizingStrategy"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.StructuredItemsView.ItemSizingStrategy"
declaring_type: "StructuredItemsView"
member_kind: property
---

# StructuredItemsView.ItemSizingStrategy

> [!abstract] Property of [[StructuredItemsView|StructuredItemsView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the strategy used to measure and size items in the view.

## Signature

```csharp
Microsoft.Maui.Controls.ItemSizingStrategy ItemSizingStrategy { get; set; }
```

## Remarks

The sizing strategy affects performance and layout behavior. Use `MeasureAllItems` to measure each item individually for accurate sizing, or `MeasureFirstItem` to use the first item's size as a template for better performance with uniform items.

## See also

- Declaring type: [[StructuredItemsView|StructuredItemsView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
