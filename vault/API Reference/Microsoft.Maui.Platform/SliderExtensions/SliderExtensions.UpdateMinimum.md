---
title: "SliderExtensions.UpdateMinimum"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.SliderExtensions.UpdateMinimum"
declaring_type: "SliderExtensions"
member_kind: method
---

# SliderExtensions.UpdateMinimum

> [!abstract] Method of [[SliderExtensions|SliderExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the minimum value of the native slider to match the specified cross-platform slider.

## Signatures

```csharp
void static UpdateMinimum(this Android.Widget.SeekBar! seekBar, Microsoft.Maui.ISlider! slider)
void static UpdateMinimum(this UIKit.UISlider! uiSlider, Microsoft.Maui.ISlider! slider)
void static UpdateMinimum(this Tizen.NUI.Components.Slider! platformSlider, Microsoft.Maui.ISlider! slider)
void static UpdateMinimum(this Microsoft.UI.Xaml.Controls.Slider! nativeSlider, Microsoft.Maui.ISlider! slider)
```

## See also

- Declaring type: [[SliderExtensions|SliderExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
