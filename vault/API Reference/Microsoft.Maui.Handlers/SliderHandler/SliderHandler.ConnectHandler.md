---
title: "SliderHandler.ConnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.SliderHandler.ConnectHandler"
declaring_type: "SliderHandler"
member_kind: method
---

# SliderHandler.ConnectHandler

> [!abstract] Method of [[SliderHandler|SliderHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Attaches the handler to the native slider control and wires up platform event handling.

## Signatures

```csharp
void override ConnectHandler(Android.Widget.SeekBar! platformView)
void override ConnectHandler(UIKit.UISlider! platformView)
void override ConnectHandler(Tizen.NUI.Components.Slider! platformView)
void override ConnectHandler(Microsoft.UI.Xaml.Controls.Slider! platformView)
```

## See also

- Declaring type: [[SliderHandler|SliderHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
