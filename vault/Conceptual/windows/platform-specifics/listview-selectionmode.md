---
title: "ListView SelectionMode on Windows"
description: "This article explains how to consume the .NET MAUI Windows platform-specific that controls whether items in a ListView can respond to tap gestures."
tags:
  - conceptual
  - area/windows
ms_date: "04/06/2022"
source: "https://learn.microsoft.com/dotnet/maui/windows/platform-specifics/listview-selectionmode?view=net-maui-10.0"
---

# ListView SelectionMode on Windows

On Windows, by default the .NET Multi-platform App UI (.NET MAUI) [[ListView (Controls)|ListView]] uses the native `ItemClick` event to respond to interaction, rather than the native `Tapped` event. This provides accessibility functionality so that the Windows Narrator and the keyboard can interact with the [[ListView (Controls)|ListView]]. However, it also renders any tap gestures inside the [[ListView (Controls)|ListView]] inoperable.

This .NET MAUI Windows platform-specific controls whether items in a [[ListView (Controls)|ListView]] can respond to tap gestures, and hence whether the native [[ListView (Controls)|ListView]] fires the `ItemClick` or `Tapped` event. It's consumed in XAML by setting the `ListView.SelectionMode` attached property to a value of the `ListViewSelectionMode` enumeration:

```xaml
<ContentPage ...
             xmlns:windows="clr-namespace:Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific;assembly=Microsoft.Maui.Controls">
    <StackLayout>
        <ListView ... windows:ListView.SelectionMode="Inaccessible">
            ...
        </ListView>
    </StackLayout>
</ContentPage>
```

Alternatively, it can be consumed from C# using the fluent API:

```csharp
using Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific;
...

listView.On<Microsoft.Maui.Controls.PlatformConfiguration.Windows>().SetSelectionMode(ListViewSelectionMode.Inaccessible);
```

The `ListView.On<Microsoft.Maui.Controls.PlatformConfiguration.Windows>` method specifies that this platform-specific will only run on Windows. The `ListView.SetSelectionMode` method, in the `Microsoft.Maui.Controls.PlatformConfiguration.WindowsSpecific` namespace, is used to control whether items in a [[ListView (Controls)|ListView]] can respond to tap gestures, with the `ListViewSelectionMode` enumeration providing two possible values:

- `Accessible` – indicates that the [[ListView (Controls)|ListView]] will fire the native `ItemClick` event to handle interaction, and hence provide accessibility functionality. Therefore, the Windows Narrator and the keyboard can interact with the [[ListView (Controls)|ListView]]. However, items in the [[ListView (Controls)|ListView]] can't respond to tap gestures. This is the default behavior for [[ListView (Controls)|ListView]] objects on Windows.
- `Inaccessible` – indicates that the [[ListView (Controls)|ListView]] will fire the native `Tapped` event to handle interaction. Therefore, items in the [[ListView (Controls)|ListView]] can respond to tap gestures. However, there's no accessibility functionality and hence the Windows Narrator and the keyboard can't interact with the [[ListView (Controls)|ListView]].

> [!NOTE]
> The `Accessible` and `Inaccessible` selection modes are mutually exclusive, and you will need to choose between an accessible [[ListView (Controls)|ListView]] or a [[ListView (Controls)|ListView]] that can respond to tap gestures.

In addition, the `GetSelectionMode` method can be used to return the current `ListViewSelectionMode`.

The result is that a specified `ListViewSelectionMode` is applied to the [[ListView (Controls)|ListView]], which controls whether items in the [[ListView (Controls)|ListView]] can respond to tap gestures, and hence whether the native [[ListView (Controls)|ListView]] fires the `ItemClick` or `Tapped` event.
