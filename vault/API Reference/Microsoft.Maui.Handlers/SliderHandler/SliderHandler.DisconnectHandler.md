---
title: "SliderHandler.DisconnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.SliderHandler.DisconnectHandler"
declaring_type: "SliderHandler"
member_kind: method
---

# SliderHandler.DisconnectHandler

> [!abstract] Method of [[SliderHandler|SliderHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Detaches the handler from the native slider control and releases associated resources.

## Signatures

```csharp
void override DisconnectHandler(Android.Widget.SeekBar! platformView)
void override DisconnectHandler(UIKit.UISlider! platformView)
void override DisconnectHandler(Tizen.NUI.Components.Slider! platformView)
void override DisconnectHandler(Microsoft.UI.Xaml.Controls.Slider! platformView)
```

## See also

- Declaring type: [[SliderHandler|SliderHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
