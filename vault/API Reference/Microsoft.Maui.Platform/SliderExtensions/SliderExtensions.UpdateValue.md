---
title: "SliderExtensions.UpdateValue"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.SliderExtensions.UpdateValue"
declaring_type: "SliderExtensions"
member_kind: method
---

# SliderExtensions.UpdateValue

> [!abstract] Method of [[SliderExtensions|SliderExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Updates the current value of the native slider to match the specified cross-platform slider.

## Signatures

```csharp
void static UpdateValue(this Android.Widget.SeekBar! seekBar, Microsoft.Maui.ISlider! slider)
void static UpdateValue(this UIKit.UISlider! uiSlider, Microsoft.Maui.ISlider! slider)
void static UpdateValue(this Tizen.NUI.Components.Slider! platformSlider, Microsoft.Maui.ISlider! slider)
void static UpdateValue(this Microsoft.UI.Xaml.Controls.Slider! nativeSlider, Microsoft.Maui.ISlider! slider)
```

## See also

- Declaring type: [[SliderExtensions|SliderExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
