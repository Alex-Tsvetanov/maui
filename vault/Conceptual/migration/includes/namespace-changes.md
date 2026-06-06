---
title: "Namespace Changes"
tags:
  - conceptual
  - area/migration
ms_date: "08/30/2023"
source: "https://learn.microsoft.com/dotnet/maui/migration/includes/namespace-changes?view=net-maui-10.0"
---

## Namespace changes

Namespaces have changed in the move from Xamarin.Forms to .NET MAUI, and Xamarin.Essentials features are now part of .NET MAUI. To make namespace updates, perform a find and replace for the following namespaces:

| Xamarin.Forms namespace    | .NET MAUI namespaces                                               |
|----------------------------|--------------------------------------------------------------------|
| `Xamarin.Forms`            | `Maui` and `Controls`           |
| `Xamarin.Forms.DualScreen` | `Foldable`                            |
| `Xamarin.Forms.Maps`       | `Maps` and `Maps` |
| `Xamarin.Forms.PlatformConfiguration` | `PlatformConfiguration`    |
| `Xamarin.Forms.PlatformConfiguration.AndroidSpecific` | `AndroidSpecific` |
| `Xamarin.Forms.PlatformConfiguration.AndroidSpecific.AppCompat` | `AppCompat` |
| `Xamarin.Forms.PlatformConfiguration.TizenSpecific` | `TizenSpecific` |
| `Xamarin.Forms.PlatformConfiguration.WindowsSpecific` | `WindowsSpecific` |
| `Xamarin.Forms.PlatformConfiguration.iOSSpecific` | `iOSSpecific` |
| `Xamarin.Forms.Shapes` | `Shapes` |
| `Xamarin.Forms.StyleSheets` | `StyleSheets` |
| `Xamarin.Forms.Xaml` | `Xaml` |

.NET MAUI projects make use of implicit `global using` directives. This feature enables you to remove `using` directives for the `Xamarin.Essentials` namespace, without having to replace them with the equivalent .NET MAUI namespaces.

In addition, the default XAML namespace has changed from `http://xamarin.com/schemas/2014/forms` in Xamarin.Forms to `http://schemas.microsoft.com/dotnet/2021/maui` in .NET MAUI. Therefore, you should replace all occurrences of `xmlns="http://xamarin.com/schemas/2014/forms"` with `xmlns="http://schemas.microsoft.com/dotnet/2021/maui"`.

> [!NOTE]
> You can quickly update your `Xamarin.Forms` namespaces to `Microsoft.Maui` by using [[upgrade-assistant#quick-actions-in-visual-studio|Quick actions in Visual Studio]], provided that you have [[upgrade-assistant|Upgrade Assistant]] installed.
