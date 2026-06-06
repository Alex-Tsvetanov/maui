---
title: "ListView row animations on iOS"
description: "This article explains how to consume the .NET MAUI iOS platform-specific that controls whether row animations are disabled when the ListView items collection is being updated."
tags:
  - conceptual
  - area/ios
ms_date: "04/05/2022"
source: "https://learn.microsoft.com/dotnet/maui/ios/platform-specifics/listview-row-animations?view=net-maui-10.0"
---

# ListView row animations on iOS

This .NET Multi-platform App UI (.NET MAUI) iOS platform-specific controls whether row animations are disabled when the [[ListView (Controls)|ListView]] items collection is being updated. It's consumed in XAML by setting the `ListView.RowAnimationsEnabled` bindable property to `false`:

```xaml
<ContentPage ...
             xmlns:ios="clr-namespace:Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;assembly=Microsoft.Maui.Controls">
    <StackLayout>
        <ListView ... ios:ListView.RowAnimationsEnabled="false">
            ...
        </ListView>
    </StackLayout>
</ContentPage>
```

Alternatively, it can be consumed from C# using the fluent API:

```csharp
using Microsoft.Maui.Controls.PlatformConfiguration;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;
...

listView.On<iOS>().SetRowAnimationsEnabled(false);
```

The `ListView.On<iOS>` method specifies that this platform-specific will only run on iOS. The `ListView.SetRowAnimationsEnabled` method, in the `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific` namespace, is used to control whether row animations are disabled when the [[ListView (Controls)|ListView]] items collection is being updated. In addition, the `ListView.GetRowAnimationsEnabled` method can be used to return whether row animations are disabled on the [[ListView (Controls)|ListView]].

> [!NOTE]
> [[ListView (Controls)|ListView]] row animations are enabled by default. Therefore, an animation occurs when a new row is inserted into a [[ListView (Controls)|ListView]].
