---
title: "SliderExtensions.UpdateMaximum"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.SliderExtensions.UpdateMaximum"
declaring_type: "SliderExtensions"
member_kind: method
---

# SliderExtensions.UpdateMaximum

> [!abstract] Method of [[SliderExtensions|SliderExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the maximum value of the native slider to match the specified cross-platform slider.

## Signatures

```csharp
void static UpdateMaximum(this Android.Widget.SeekBar! seekBar, Microsoft.Maui.ISlider! slider)
void static UpdateMaximum(this UIKit.UISlider! uiSlider, Microsoft.Maui.ISlider! slider)
void static UpdateMaximum(this Tizen.NUI.Components.Slider! platformSlider, Microsoft.Maui.ISlider! slider)
void static UpdateMaximum(this Microsoft.UI.Xaml.Controls.Slider! nativeSlider, Microsoft.Maui.ISlider! slider)
```

## See also

- Declaring type: [[SliderExtensions|SliderExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
