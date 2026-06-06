---
title: "CarouselView"
description: "The .NET MAUI CarouselView is a view for presenting data in a scrollable layout, where users can swipe to move through a collection of items."
tags:
  - conceptual
  - area/user-interface
ms_date: "08/19/2025"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/carouselview?view=net-maui-10.0"
---

# CarouselView

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-carouselview)

The .NET Multi-platform App UI (.NET MAUI) [[CarouselView|CarouselView]] is a view for presenting data in a scrollable layout, where users can swipe to move through a collection of items.

By default, [[CarouselView|CarouselView]] will display its items in a horizontal orientation. A single item will be displayed on screen, with swipe gestures resulting in forwards and backwards navigation through the collection of items. In addition, indicators can be displayed that represent each item in the [[CarouselView|CarouselView]]:

![](media/populate-data/indicators.png)

By default, [[CarouselView|CarouselView]] provides looped access to its collection of items. Therefore, swiping backwards from the first item in the collection will display the last item in the collection. Similarly, swiping forwards from the last item in the collection will return to the first item in the collection.

[[CarouselView|CarouselView]] shares much of its implementation with [[CollectionView|CollectionView]]. However, the two controls have different use cases. [[CollectionView|CollectionView]] is typically used to present lists of data of any length, whereas [[CarouselView|CarouselView]] is typically used to highlight information in a list of limited length. For more information about [[CollectionView|CollectionView]], see [[collectionview|CollectionView]].


> [!NOTE]
> On iOS and Mac Catalyst, the optimized handlers that were optional in .NET 9 are the default handlers for [[CarouselView|CarouselView]] in .NET 10, providing improved performance and stability.

## Revert to .NET 9 behavior

We recommend using the new handler for [[CarouselView|CarouselView]], but if you would like to opt-out of this behavior and revert back to the .NET 9 handler, you can use the code below in your `MauiProgram.cs`.

```csharp
#if IOS || MACCATALYST
builder.ConfigureMauiHandlers(handlers =>
{
    handlers.AddHandler<Microsoft.Maui.Controls.CarouselView, Microsoft.Maui.Controls.Handlers.Items.CarouselViewHandler>();
});
#endif
```

