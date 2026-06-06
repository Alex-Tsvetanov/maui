---
title: "CarouselView.VisibleViews"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.CarouselView.VisibleViews"
declaring_type: "CarouselView"
member_kind: property
---

# CarouselView.VisibleViews

> [!abstract] Property of [[CarouselView|CarouselView]]
> Namespace: `Microsoft.Maui.Controls`

Gets the collection of views currently visible in the carousel.

## Signature

```csharp
System.Collections.ObjectModel.ObservableCollection<Microsoft.Maui.Controls.View> VisibleViews { get; }
```

## Remarks

This collection is automatically updated as the user scrolls through the carousel. It includes the current item and any partially visible adjacent items based on `PeekAreaInsets`.

## See also

- Declaring type: [[CarouselView|CarouselView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
