---
title: "Trimming Incompatibilities"
tags:
  - conceptual
  - area/deployment
ms_date: "12/03/2024"
source: "https://learn.microsoft.com/dotnet/maui/deployment/includes/trimming-incompatibilities?view=net-maui-10.0"
---

The following .NET MAUI features are incompatible with full trimming and will be removed by the trimmer:

- Binding expressions where that binding path is set to a string. Instead, use compiled bindings. For more information, see [[compiled-bindings|Compiled bindings]].
- Implicit conversion operators, when assigning a value of an incompatible type to a property in XAML, or when two properties of different types use a data binding. Instead, you should define a `TypeConverter` for your type and attach it to the type using the `TypeConverterAttribute`. For more information, see [[trimming#define-a-typeconverter-to-replace-an-implicit-conversion-operator|Define a TypeConverter to replace an implicit conversion operator]].
- Loading XAML at runtime with the `LoadFromXaml%2A` extension method. This XAML can be made trim safe by annotating all types that could be loaded at runtime with the `DynamicallyAccessedMembers` attribute or the `DynamicDependency` attribute. However, this is very error prone and isn't recommended.
- Receiving navigation data using the [[QueryPropertyAttribute|QueryPropertyAttribute]]. Instead, you should implement the [[IQueryAttributable|IQueryAttributable]] interface on types that need to accept query parameters. For more information, see [[navigation#process-navigation-data-using-a-single-method|Process navigation data using a single method]].
- The `SearchHandler.DisplayMemberName` property. Instead, you should provide an [[ItemsView{TVisual}.ItemTemplate|ItemTemplate]] to define the appearance of [[SearchHandler|SearchHandler]] results. For more information, see [[search#define-search-results-item-appearance|Define search results item appearance]].
- The [[HybridWebView|HybridWebView]] control, due to its use of dynamic `System.Text.Json` serialization features.
- UI customization with the [[OnPlatformExtension|`OnPlatform`]] XAML markup extension. Instead, you should use the `OnPlatform`1` class. For more information, see [[customize-ui-appearance#customize-ui-appearance-based-on-the-platform|Customize UI appearance based on the platform]].
- UI customization with the [[OnIdiomExtension|`OnIdiom`]] XAML markup extension. Instead, you should use the `OnIdiom`1` class. For more information, see [[customize-ui-appearance#customize-ui-appearance-based-on-the-device-idiom|Customize UI appearance based on the device idiom]].
