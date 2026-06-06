---
title: "Compile Troubleshoot"
tags:
  - conceptual
  - area/migration
ms_date: "10/06/2023"
source: "https://learn.microsoft.com/dotnet/maui/migration/includes/compile-troubleshoot?view=net-maui-10.0"
---

## Compile and troubleshoot

Once your dependencies are resolved, you should build your project. Any errors will guide you towards next steps.

<!-- markdownlint-disable MD032 -->
> [!TIP]
> - Delete all *bin* and *obj* folders from all projects before opening and building projects in Visual Studio, particularly when changing .NET versions.
> - Delete the *Resource.designer.cs* generated file from the Android project.
<!-- markdownlint-enable MD032 -->

The following table provides guidance for overcoming common build or runtime issues:

| Issue | Tip |
| ----- | --- |
| `Xamarin.*` namespace doesn't exist. | Update the namespace to its .NET MAUI equivalent. For more information, see [Namespace changes](#namespace-changes). |
| API doesn't exist. | Update the API usage to its .NET MAUI equivalent. For more information, see [API changes](#api-changes). |
| App won't deploy. | Ensure that the required platform project is set to deploy in Visual Studio's Configuration Manager. |
| App won't launch. | Update each platform project's entry point class, and the app entry point. For more information, see [[multi-project-to-multi-project#bootstrap-your-migrated-app|Bootstrap your migrated app]]. |
| [[CollectionView|CollectionView]] doesn't scroll. | Check the container layout and the measured size of the [[CollectionView|CollectionView]]. By default the control will take up as much space as the container allows. A [[Grid (Controls)|Grid]] constrains children at its own size. However a [[StackLayout (Controls)|StackLayout]] enables children to take up space beyond its bounds. |
| Pop-up is displayed under the page on iOS. | In Xamarin.Forms, all pop-ups on iOS are `UIWindow` instances but in .NET MAUI pop-ups are displayed by locating the current presenting `ViewController` and displaying the pop-up with `PresentViewControllerAsync`. In plugins such as Mopups, to ensure that your pop-ups are correctly displayed you should call `DisplayAlert%2A` (or `DisplayAlertAsync%2A` in .NET 10+), `DisplayActionSheet%2A` (or `DisplayActionSheetAsync%2A` in .NET 10+), or `DisplayPromptAsync%2A` from the [[ContentPage|ContentPage]] that's used inside the `Mopup` popup. |
| [[BoxView (Controls)|BoxView]] not appearing. | The default size of a [[BoxView (Controls)|BoxView]] in Xamarin.Forms is 40x40. The default size of a [[BoxView (Controls)|BoxView]] in .NET MAUI is 0x0. Set `WidthRequest` and `HeightRequest` to 40. |
| Layout is missing padding, margin, or spacing. | Add default values to your project based on the .NET MAUI style resource. For more information, see [[layouts#default-layout-value-changes-from-xamarinforms|Default value changes from Xamarin.Forms]]. |
| Custom layout doesn't work. | Custom layout code needs updating to work in .NET MAUI. For more information, see [Custom layout changes](#custom-layout-changes). |
| Custom renderer doesn't work. | Renderer code needs updating to work in .NET MAUI. For more information, see [[custom-renderers|Use custom renderers in .NET MAUI]]. |
| Effect doesn't work. | Effect code needs updating to work in .NET MAUI. For more information, see [[effects|Use effects in .NET MAUI]]. |
| SkiaSharp code doesn't work. | SkiaSharp code needs minor updates to work in .NET MAUI. For more information, see [[skiasharp|Reuse SkiaSharp code in .NET MAUI]]. |
| Can't access previously created app properties data. | Migrate the app properties data to .NET MAUI preferences. For more information, see [[app-properties|Migrate data from the Xamarin.Forms app properties dictionary to .NET MAUI preferences]]. |
| Can't access previously created secure storage data. | Migrate the secure storage data to .NET MAUI. For more information, see [[secure-storage|Migrate from Xamarin.Essentials secure storage to .NET MAUI secure storage]]. |
| Can't access previously created version tracking data. | Migrate the version tracking data to .NET MAUI. For more information, see [[version-tracking|Migrate version tracking data from a Xamarin.Forms app to a .NET MAUI app]]. |
