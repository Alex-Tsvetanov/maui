---
title: "AppHostBuilderExtensions (Hosting).UseMauiMaps"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Hosting
aliases:
  - "Microsoft.Maui.Controls.Hosting.AppHostBuilderExtensions.UseMauiMaps"
declaring_type: "AppHostBuilderExtensions (Hosting)"
member_kind: method
---

# AppHostBuilderExtensions (Hosting).UseMauiMaps

> [!abstract] Method of [[AppHostBuilderExtensions (Hosting)|AppHostBuilderExtensions (Hosting)]]
> Namespace: `Microsoft.Maui.Controls.Hosting`

Configures `MauiAppBuilder` to add support for the `Map` control.

## Signature

```csharp
Microsoft.Maui.Hosting.MauiAppBuilder! static UseMauiMaps(this Microsoft.Maui.Hosting.MauiAppBuilder! builder)
```

## Parameters

| Parameter | Description |
|---|---|
| `builder` | The `MauiAppBuilder` to configure. |

## Returns

The configured `MauiAppBuilder`.

## Remarks

Windows (Azure Maps): Set your Azure Maps subscription key using ConfigureEssentials : builder.ConfigureEssentials(essentials => essentials.UseMapServiceToken("YOUR_AZURE_MAPS_KEY")); Get a key from the Azure Portal: https://portal.azure.com → Azure Maps account → Authentication Windows Features (via Azure Maps JS API): The Windows implementation uses the WinUI 3 MapControl backed by Azure Maps. The following features are implemented by accessing the Azure Maps JavaScript API through the control's internal WebView2: MoveToRegion: Navigates via map.setCamera() . MapType: Street/Satellite/Hybrid via map.setStyle() . IsTrafficEnabled: Traffic flow and incidents via map.setTraffic() . IsScrollEnabled/IsZoomEnabled: Independent control via map.setUserInteraction() . Pins: Via MapIcon on a MapElementsLayer . Windows Platform Limitations: User Location: Not built-in; requires manual Geolocation API integration. Shapes: Polylines, polygons, and circles are not supported ( MapElementsLayer only supports MapIcon ). Pin Labels: MapIcon does not support labels or info windows. Map.Clicked (background): Only MapElement clicks fire events, not empty map area clicks. See `MapHandler` documentation for detailed platform information.

## See also

- Declaring type: [[AppHostBuilderExtensions (Hosting)|AppHostBuilderExtensions (Hosting)]]
- [[_Microsoft.Maui.Controls.Hosting|Microsoft.Maui.Controls.Hosting namespace]]
