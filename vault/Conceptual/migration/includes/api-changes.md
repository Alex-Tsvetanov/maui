---
title: "Api Changes"
tags:
  - conceptual
  - area/migration
ms_date: "08/30/2023"
source: "https://learn.microsoft.com/dotnet/maui/migration/includes/api-changes?view=net-maui-10.0"
---

## API changes

Some APIs have changed in the move from Xamarin.Forms to .NET MAUI. This is multiple reasons including removing duplicate functionality caused by Xamarin.Essentials becoming part of .NET MAUI, and ensuring that APIs follow .NET naming guidelines. The following sections discuss these changes.

### Color changes

In Xamarin.Forms, the `Xamarin.Forms.Color` struct lets you construct [[Color|Color]] objects using `double` values, and provides named colors, such as `Xamarin.Forms.Color.AliceBlue`. In .NET MAUI, this functionality has been separated into the [[Color|Color]] class, and the [[Colors|Colors]] class.

The [[Color|Color]] class, in the `Graphics` namespace, lets you construct [[Color|Color]] objects using `float` values, `byte` values, and `int` values. The [[Colors|Colors]] class, which is also in the `Graphics` namespace, largely provides the same named colors. For example, use [[Colors.AliceBlue|Colors.AliceBlue]] to specify the `AliceBlue` color.

The following table shows the API changes between the `Xamarin.Forms.Color` struct and the [[Color|Color]] class:

| Xamarin.Forms API       | .NET MAUI API                                                       | Comment |
|-------------------------|---------------------------------------------------------------------|---------|
| `Xamarin.Forms.Color.R` | [[Color.Red|Red]]   |         |
| `Xamarin.Forms.Color.G` | [[Color.Green|Green]] |         |
| `Xamarin.Forms.Color.B` | [[Color.Blue|Blue]]  |         |
| `Xamarin.Forms.Color.A` | [[Color.Alpha|Alpha]] |         |
| `Xamarin.Forms.Color.Hue` | `GetHue%2A` | Xamarin.Forms property replaced with a method in .NET MAUI. |
| `Xamarin.Forms.Color.Saturation` | `GetSaturation%2A` | Xamarin.Forms property replaced with a method in .NET MAUI. |
| `Xamarin.Forms.Color.Luminosity` | `GetLuminosity%2A` | Xamarin.Forms property replaced with a method in .NET MAUI. |
| `Xamarin.Forms.Color.Default` | | No .NET MAUI equivalent. Instead, [[Color|Color]] objects default to `null`. |
| `Xamarin.Forms.Color.Accent` |  | No .NET MAUI equivalent. |
| `Xamarin.Forms.Color.FromHex` | `FromArgb%2A` | `FromHex%2A` is obsolete and will be removed in a future release. |

In addition, all of the numeric values in a [[Color|Color]] are `float`, rather than `double` as used in `Xamarin.Forms.Color`.

> [!NOTE]
> Unlike Xamarin.Forms, a [[Color|Color]] doesn't have an implicit conversion to `Color`.

### Layout changes

The following table lists the layout APIs that have been removed in the move from Xamarin.Forms to .NET MAUI:

| Xamarin.Forms API                                   | .NET MAUI API | Comments                 |
|-----------------------------------------------------|---------------|--------------------------|
| `Xamarin.Forms.AbsoluteLayout.IAbsoluteList<T>.Add` |               | The `Add` overload that accepts three arguments isn't present in .NET MAUI. |
| `Xamarin.Forms.Grid.IGridList<T>.AddHorizontal`     |               | No .NET MAUI equivalent. |
| `Xamarin.Forms.Grid.IGridList<T>.AddVertical`       |               | No .NET MAUI equivalent. |
| `Xamarin.Forms.RelativeLayout` | [[RelativeLayout|RelativeLayout]] | In .NET MAUI, `RelativeLayout` only exists as a compatibility control for users migrating from Xamarin.Forms. Use [[Grid (Controls)|Grid]] instead, or add the `xmlns` for the compatibility namespace. |

In addition, adding children to a layout in code in Xamarin.Forms is accomplished by adding the children to the layout's `Children` collection:

```csharp
Grid grid = new Grid();
grid.Children.Add(new Label { Text = "Hello world" });
```

In .NET MAUI, the [[Layout (Controls).Children|Children]] collection is for internal use by .NET MAUI and shouldn't be manipulated directly. Therefore, in code children should be added directly to the layout:

```csharp
Grid grid = new Grid();
grid.Add(new Label { Text = "Hello world" });
```

> [!IMPORTANT]
> Any `Add` layout extension methods, such as `GridExtensions.Add%2A`, are invoked on the layout rather than the layouts [[Layout (Controls).Children|Children]] collection.

You may notice when running your upgraded .NET MAUI app that layout behavior is different. For more information, see [[layouts|Layout behavior changes from Xamarin.Forms]].

### Custom layout changes

The process for creating a custom layout in Xamarin.Forms involves creating a class that derives from `Layout<View>`, and overriding the `VisualElement.OnMeasure` and `Layout.LayoutChildren` methods. For more information, see [Create a custom layout in Xamarin.Forms](/xamarin/xamarin-forms/user-interface/layouts/custom).

In .NET MAUI, the layout classes derive from the abstract [[Layout (Controls)|Layout]] class. This class delegates cross-platform layout and measurement to a layout manager class. Each layout manager class implements the [[ILayoutManager|ILayoutManager]] interface, which specifies that `Measure%2A` and `ArrangeChildren%2A` implementations must be provided:

- The `Measure%2A` implementation calls `IView.Measure%2A` on each view in the layout, and returns the total size of the layout given the constraints.
- The `ArrangeChildren%2A` implementation determines where each view should be placed within the bounds of the layout, and calls `Arrange%2A` on each view with its appropriate bounds. The return value is the actual size of the layout.

For more information, see [[custom|Custom layouts]].

### Device changes

Xamarin.Forms has a `Xamarin.Forms.Device` class that helps you to interact with the device and platform the app is running on. The equivalent class in .NET MAUI, [[Device|Device]], is deprecated and its functionality is replaced by multiple types.

The following table shows the .NET MAUI replacements for the functionality in the `Xamarin.Forms.Device` class:

| Xamarin.Forms API | .NET MAUI API | Comments |
| ----------------- | ------------- | -------- |
| `Xamarin.Forms.Device.Android` | [[DevicePlatform.Android|Android]] |  |
| `Xamarin.Forms.Device.iOS` | [[DevicePlatform.iOS|iOS]] | |
| `Xamarin.Forms.Device.GTK` |  | No .NET MAUI equivalent. |
| `Xamarin.Forms.Device.macOS` | | No .NET MAUI equivalent. Instead, use [[DevicePlatform.MacCatalyst|MacCatalyst]]. |
| `Xamarin.Forms.Device.Tizen` | [[DevicePlatform.Tizen|Tizen]] | |
| `Xamarin.Forms.Device.UWP` | [[DevicePlatform.WinUI|WinUI]] | |
| `Xamarin.Forms.Device.WPF` |  |  No .NET MAUI equivalent. |
| `Xamarin.Forms.Device.Flags` | | No .NET MAUI equivalent. |
| `Xamarin.Forms.Device.FlowDirection` | [[AppInfo.RequestedLayoutDirection|RequestedLayoutDirection]] | |
| `Xamarin.Forms.Device.Idiom` | [[DeviceInfo.Idiom|Idiom]] | |
| `Xamarin.Forms.Device.IsInvokeRequired` | [[Dispatcher.IsDispatchRequired|IsDispatchRequired]] | |
| `Xamarin.Forms.Device.OS` | [[DeviceInfo.Platform|Platform]] | |
| `Xamarin.Forms.Device.RuntimePlatform` | [[DeviceInfo.Platform|Platform]] | |
| `Xamarin.Forms.Device.BeginInvokeOnMainThread` | `BeginInvokeOnMainThread%2A` | |
| `Xamarin.Forms.Device.GetMainThreadSynchronizationContextAsync` | `GetMainThreadSynchronizationContextAsync%2A` | |
| `Xamarin.Forms.Device.GetNamedColor` | | No .NET MAUI equivalent. |
| `Xamarin.Forms.Device.GetNamedSize` | | No .NET MAUI equivalent.|
| `Xamarin.Forms.Device.Invalidate`  | `InvalidateMeasure%2A` | |
| `Xamarin.Forms.Device.InvokeOnMainThreadAsync` | `InvokeOnMainThreadAsync%2A` | |
| `Xamarin.Forms.Device.OnPlatform` | [[DeviceInfo.Platform|Platform]] | |
| `Xamarin.Forms.Device.OpenUri` | `OpenAsync%2A` | |
| `Xamarin.Forms.Device.SetFlags` | | No .NET MAUI equivalent. |
| `Xamarin.Forms.Device.SetFlowDirection` | [[Window.FlowDirection|FlowDirection]] | |
| `Xamarin.Forms.Device.StartTimer` | `StartTimer%2A` or `DispatchDelayed%2A` | |

### Map changes

In Xamarin.Forms, the `Map` control and associated types are in the `Xamarin.Forms.Maps` namespace. In .NET MAUI, this functionality has moved to the `Maps` and `Maps` namespaces. Some properties have been renamed and some types have been replaced with equivalent types from Xamarin.Essentials.

The following table shows the .NET MAUI replacements for the functionality in the `Xamarin.Forms.Maps` namespace:

| Xamarin.Forms API | .NET MAUI API | Comment |
| ----------------- | ------------- | ------- |
| `Xamarin.Forms.Maps.Map.HasScrollEnabled` | [[Map (Maps).IsScrollEnabled|IsScrollEnabled]] |  |
| `Xamarin.Forms.Maps.Map.HasZoomEnabled` | [[Map (Maps).IsZoomEnabled|IsZoomEnabled]] |  |
| `Xamarin.Forms.Maps.Map.TrafficEnabled` | [[Map (Maps).IsTrafficEnabled|IsTrafficEnabled]] |  |
| `Xamarin.Forms.Maps.Map.MoveToLastRegionOnLayoutChange` |  | No .NET MAUI equivalent. |
| `Xamarin.Forms.Maps.Pin.Id` | [[Pin.MarkerId|MarkerId]] |  |
| `Xamarin.Forms.Maps.Pin.Position` | [[Pin.Location|Location]] |  |
| `Xamarin.Forms.Maps.MapClickedEventArgs.Position` | [[MapClickedEventArgs.Location|Location]] |  |
| `Xamarin.Forms.Maps.Position` | [[Location|Location]] | Members of type `Xamarin.Forms.Maps.Position` have changed to the [[Location|Location]] type. |
| `Xamarin.Forms.Maps.Geocoder` | [[Geocoding|Geocoding]] | Members of type `Xamarin.Forms.Maps.Geocoder` have changed to the [[Geocoding|Geocoding]] type. |

.NET MAUI has two `Map` types - [[Map (Maps)|Map]] and [[Map (ApplicationModel)|Map]]. Because the `ApplicationModel` namespace is one of .NET MAUI's `global using` directives, when using the [[Map (Maps)|Map]] control from code you'll have to fully qualify your `Map` usage or use a [using alias](/dotnet/csharp/language-reference/keywords/using-directive#using-alias).

In XAML, an `xmlns` namespace definition should be added for the [[Map (Maps)|Map]] control. While this isn't required, it prevents a collision between the `Polygon` and `Polyline` types, which exist in both the `Maps` and `Shapes` namespaces. For more information, see [[map#display-a-map|Display a map]].

### Other changes

A small number of other APIs have been consolidated in the move from Xamarin.Forms to .NET MAUI. The following table shows these changes:

| Xamarin.Forms API | .NET MAUI API | Comments |
| ----------------- | ------------- | -------- |
| `Xamarin.Forms.Application.Properties` | [[Preferences|Preferences]] |  |
| `Xamarin.Forms.Button.Image` | [[Button (Controls).ImageSource|ImageSource]] |  |
| `Xamarin.Forms.Frame.OutlineColor` | [[Frame.BorderColor|BorderColor]] |  |
| `Xamarin.Forms.IQueryAttributable.ApplyQueryAttributes` | `ApplyQueryAttributes%2A` | In Xamarin.Forms, the `ApplyQueryAttributes` method accepts an `IDictionary<string, string>` argument. In .NET MAUI, the `ApplyQueryAttributes` method accepts an `IDictionary<string, object>` argument.  |
| `Xamarin.Forms.MenuItem.Icon` | [[MenuItem.IconImageSource|IconImageSource]] | `Xamarin.Forms.MenuItem.Icon` is the base class for `Xamarin.Forms.ToolbarItem`, and so `ToolbarItem.Icon` becomes `ToolbarItem.IconImageSource`. |
| `Xamarin.Forms.OrientationStateTrigger.Orientation` | [[OrientationStateTrigger.Orientation|Orientation]] | In Xamarin.Forms, the `OrientationStateTrigger.Orientation` property is of type `Xamarin.Forms.Internals.DeviceOrientation`. In .NET MAUI, the `OrientationStateTrigger.Orientation` property is of type [[DisplayOrientation|DisplayOrientation]]. |
| `Xamarin.Forms.OSAppTheme` | [[AppTheme|AppTheme]] |  |
| `Xamarin.Forms.Span.ForegroundColor` | [[Span.TextColor|TextColor]] |  |
| `Xamarin.Forms.ToolbarItem.Name` | [[MenuItem.Text|Text]] | [[MenuItem.Text|Text]] is the base class for [[ToolbarItem|ToolbarItem]], and so `ToolbarItem.Name` becomes `ToolbarItem.Text`. |

In addition, in Xamarin.Forms, the `Page.OnAppearing` override is called on Android when an app is backgrounded and then brought to the foreground. However, this override isn't called on iOS and Windows in the same scenario. In .NET MAUI, the [[Page (Controls).OnAppearing|OnAppearing]] override isn't called on any platforms when an app is backgrounded and then brought to the foreground. Instead, you should listen to lifecycle events on [[Window|Window]] to be notified when an app returns to the foreground. For more information, see [[window|.NET MAUI windows]].

### Native forms changes

[Native forms](/xamarin/xamarin-forms/platform/native-forms) in Xamarin.Forms has become native embedding in .NET MAUI, and uses a different initialization approach and different extension methods to convert cross-platform controls to their native types. For more information, see [[native-embedding|Native embedding]].
