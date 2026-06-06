---
title: "DisplayInfo.Density"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Devices
aliases:
  - "Microsoft.Maui.Devices.DisplayInfo.Density"
declaring_type: "DisplayInfo"
member_kind: property
---

# DisplayInfo.Density

> [!abstract] Property of [[DisplayInfo|DisplayInfo]]
> Namespace: `Microsoft.Maui.Devices`

Gets a value representing the screen density.

## Signature

```csharp
double Density { get; }
```

## Remarks

The density is the scaling or a factor that can be used to convert between physical pixels and scaled pixels. For example, on high resolution displays, the physical number of pixels increases, but the scaled pixels remain the same. In a practical example for iOS, the Retina display will have a density of 2.0 or 3.0, but the units used to lay out a view does not change much. A view with a UI width of 100 may be 100 physical pixels (density = 1) on a non-Retina device, but be 200 physical pixels (density = 2) on a Retina device. On Windows, the density works similarly, and may often relate to the scale used in the display. On some monitors, the scale is set to 100% (density = 1), but on other high resolution monitors, the scale may be set to 200% (density = 2) or even 250% (density = 2.5).

## See also

- Declaring type: [[DisplayInfo|DisplayInfo]]
- [[_Microsoft.Maui.Devices|Microsoft.Maui.Devices namespace]]
