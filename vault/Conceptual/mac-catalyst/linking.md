---
title: "Linking a .NET MAUI Mac Catalyst app"
description: "Learn about the .NET for Mac Catalyst linker, which is used to eliminate unused code from a .NET MAUI Mac Catalyst app in order to reduce its size."
tags:
  - conceptual
  - area/mac-catalyst
ms_date: "08/28/2024"
source: "https://learn.microsoft.com/dotnet/maui/mac-catalyst/linking?view=net-maui-10.0"
---

# Linking a .NET MAUI Mac Catalyst app

![[linker-behavior]]

To configure linker behavior in Visual Studio Code you should add the `$(MtouchLink)` build property to a property group in your app's *.csproj* file. This build property should be set to `None`, `SdkOnly`, or `Full`:

```xml
<PropertyGroup Condition="'$(Configuration)|$(TargetFramework)|$(Platform)'=='Debug|net8.0-maccatalyst|AnyCPU'">
  <MtouchLink>SdkOnly</MtouchLink>
</PropertyGroup>
```

Alternatively, you can specify the linker behavior via the CLI when building and publishing your app. For more information, see [[deployment|Publish a .NET MAUI Mac Catalyst app]].

> [!IMPORTANT]
> The `$(MtouchLink)` build property can be set separately for each build configuration for your app.

![[linker-control]]
