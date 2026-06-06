---
title: "SliderExtensions.UpdateThumbImageSourceAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.SliderExtensions.UpdateThumbImageSourceAsync"
declaring_type: "SliderExtensions"
member_kind: method
---

# SliderExtensions.UpdateThumbImageSourceAsync

> [!abstract] Method of [[SliderExtensions|SliderExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Asynchronously updates the thumb image of the native slider from the specified cross-platform slider's image source.

## Signatures

```csharp
System.Threading.Tasks.Task! static UpdateThumbImageSourceAsync(this Android.Widget.SeekBar! seekBar, Microsoft.Maui.ISlider! slider, Microsoft.Maui.IImageSourceServiceProvider! provider)
System.Threading.Tasks.Task! static UpdateThumbImageSourceAsync(this UIKit.UISlider! uiSlider, Microsoft.Maui.ISlider! slider, Microsoft.Maui.IImageSourceServiceProvider! provider)
System.Threading.Tasks.Task! static UpdateThumbImageSourceAsync(this Tizen.NUI.Components.Slider! platformSlider, Microsoft.Maui.ISlider! slider, Microsoft.Maui.IImageSourceServiceProvider! provider)
```

## See also

- Declaring type: [[SliderExtensions|SliderExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
