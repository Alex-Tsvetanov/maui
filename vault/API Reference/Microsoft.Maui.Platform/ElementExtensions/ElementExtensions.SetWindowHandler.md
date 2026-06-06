---
title: "ElementExtensions.SetWindowHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ElementExtensions.SetWindowHandler"
declaring_type: "ElementExtensions"
member_kind: method
---

# ElementExtensions.SetWindowHandler

> [!abstract] Method of [[ElementExtensions|ElementExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Creates and assigns the cross-platform window handler for the given platform window (activity, UIWindow, or other) using the supplied MAUI context.

## Signatures

```csharp
void static SetWindowHandler(this Android.App.Activity! platformWindow, Microsoft.Maui.IWindow! window, Microsoft.Maui.IMauiContext! context)
void static SetWindowHandler(this UIKit.UIWindow! platformWindow, Microsoft.Maui.IWindow! window, Microsoft.Maui.IMauiContext! context)
void static SetWindowHandler(this Tizen.NUI.Window! platformWindow, Microsoft.Maui.IWindow! window, Microsoft.Maui.IMauiContext! context)
void static SetWindowHandler(this Microsoft.UI.Xaml.Window! platformWindow, Microsoft.Maui.IWindow! window, Microsoft.Maui.IMauiContext! context)
void static SetWindowHandler(this object! platformWindow, Microsoft.Maui.IWindow! window, Microsoft.Maui.IMauiContext! context)
```

## See also

- Declaring type: [[ElementExtensions|ElementExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
