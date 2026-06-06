---
title: "ElementExtensions.SetApplicationHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ElementExtensions.SetApplicationHandler"
declaring_type: "ElementExtensions"
member_kind: method
---

# ElementExtensions.SetApplicationHandler

> [!abstract] Method of [[ElementExtensions|ElementExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Creates and assigns the cross-platform application handler for the given platform application using the supplied MAUI context.

## Signatures

```csharp
void static SetApplicationHandler(this Android.App.Application! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.IMauiContext! context)
void static SetApplicationHandler(this UIKit.IUIApplicationDelegate! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.IMauiContext! context)
void static SetApplicationHandler(this Tizen.Applications.CoreApplication! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.IMauiContext! context)
void static SetApplicationHandler(this Microsoft.UI.Xaml.Application! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.IMauiContext! context)
void static SetApplicationHandler(this object! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.IMauiContext! context)
```

## See also

- Declaring type: [[ElementExtensions|ElementExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
