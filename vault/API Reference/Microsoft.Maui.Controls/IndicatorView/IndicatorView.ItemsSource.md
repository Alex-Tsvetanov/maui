---
title: "IndicatorView.ItemsSource"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.IndicatorView.ItemsSource"
declaring_type: "IndicatorView"
member_kind: property
---

# IndicatorView.ItemsSource

> [!abstract] Property of [[IndicatorView|IndicatorView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the collection of items for which indicators will be displayed.

## Signature

```csharp
System.Collections.IEnumerable ItemsSource { get; set; }
```

## Remarks

When set, the `Count` property is automatically updated based on the number of items. Typically bound to the same collection as a `CarouselView`'s ItemsSource property.

## See also

- Declaring type: [[IndicatorView|IndicatorView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
