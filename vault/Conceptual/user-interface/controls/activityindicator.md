---
title: "ActivityIndicator"
description: "The .NET MAUI ActivityIndicator indicates to users that the app is engaged in a lengthy activity, without giving any indication of progress."
tags:
  - conceptual
  - area/user-interface
ms_date: "03/08/2026"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/activityindicator?view=net-maui-10.0"
---

# ActivityIndicator

The .NET Multi-platform App UI (.NET MAUI) [[ActivityIndicator|ActivityIndicator]] displays an animation to show that the application is engaged in a lengthy activity. Unlike [[ProgressBar (Controls)|ProgressBar]], [[ActivityIndicator|ActivityIndicator]] gives no indication of progress.

The appearance of an [[ActivityIndicator|ActivityIndicator]] is platform-dependent, and the following screenshot shows an [[ActivityIndicator|ActivityIndicator]] on Android:

![](media/activityindicator/activityindicator-default.png)

[[ActivityIndicator|ActivityIndicator]] defines the following properties:

- `Color` is a [[Color|Color]] value that defines the color of the [[ActivityIndicator|ActivityIndicator]].
- `IsRunning` is a `bool` value that indicates whether the [[ActivityIndicator|ActivityIndicator]] should be visible and animating, or hidden. The default value of this property is `false`, which indicates that the [[ActivityIndicator|ActivityIndicator]] isn't visible.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

## Create an ActivityIndicator

To indicate a lengthy activity, create an [[ActivityIndicator|ActivityIndicator]] object and sets its properties to define its appearance.

The following XAML example shows how to display an [[ActivityIndicator|ActivityIndicator]]:

```xaml
<ActivityIndicator IsRunning="true"
                   SemanticProperties.Description="Loading" />
```

The equivalent C# code is:

```csharp
ActivityIndicator activityIndicator = new ActivityIndicator { IsRunning = true };
SemanticProperties.SetDescription(activityIndicator, "Loading");
```

The following XAML example shows how to change the color of an [[ActivityIndicator|ActivityIndicator]]:

```xaml
<ActivityIndicator IsRunning="true"
                   Color="Orange" />
```

The equivalent C# code is:

```csharp
ActivityIndicator activityIndicator = new ActivityIndicator
{
    IsRunning = true,
    Color = Colors.Orange
};
```
