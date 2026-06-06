---
title: "Display a modal page as a popover on iOS and Mac Catalyst"
description: "This article explains how to consume the .NET MAUI iOS platform-specific that enables you to display a modal page as a popover."
tags:
  - conceptual
  - area/ios
ms_date: "03/14/2025"
source: "https://learn.microsoft.com/dotnet/maui/ios/platform-specifics/page-popover?view=net-maui-10.0"
---

# Display a modal page as a popover on iOS and Mac Catalyst

This .NET Multi-platform App UI (.NET MAUI) iOS platform-specific is used to display a modal page as a popover. It's consumed by setting the `Page.ModalPopoverSourceView` bindable property to a `View`, the `Page.ModalPopoverRect` bindable property to a `Rectangle`, and the `Page.ModalPresentationStyle` bindable property to `UIModalPresentationStyle.Popover`. This can be achieved in C# using the fluent API:

```csharp
using Microsoft.Maui.Controls.PlatformConfiguration;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;
...

public partial class PopoverPage : ContentPage
{
    public PopoverPage(View modal, Rectangle rectangle)
    {
        InitializeComponent();
        On<iOS>().SetModalPopoverView(modal);
        On<iOS>().SetModalPopoverRect(rectangle);
        On<iOS>().SetModalPresentationStyle(UIModalPresentationStyle.Popover);
    }
}
```

The `Page.On<iOS>` method specifies that this platform-specific will only run on iOS. The `Page.SetModalPopoverView` method, in the `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific` namespace, is used to set the source view of the popover. The `Page.SetModalPopoverRect` method is used to set the rectangle within the view from which the popover will originate. The `Page.SetModalPresentationStyle` method is used to set the modal presentation style to popover by specifying the `UIModalPresentationStyle.Popover` enumeration value.

Pages that use this platform-specific to display a modal page as a popover must use modal navigation:

```csharp
Page modalPage = new PopoverPage(originButton, Rectangle.Empty);
await Navigation.PushModalAsync(modalPage);
```

For more information about modal navigation, see [[navigationpage#perform-modal-navigation|Perform modal navigation]].
