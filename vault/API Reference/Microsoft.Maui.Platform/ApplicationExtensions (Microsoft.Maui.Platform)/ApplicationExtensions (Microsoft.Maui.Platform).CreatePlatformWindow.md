---
title: "ApplicationExtensions (Microsoft.Maui.Platform).CreatePlatformWindow"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ApplicationExtensions.CreatePlatformWindow"
declaring_type: "ApplicationExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ApplicationExtensions (Microsoft.Maui.Platform).CreatePlatformWindow

> [!abstract] Method of [[ApplicationExtensions (Microsoft.Maui.Platform)|ApplicationExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Creates the platform window for the specified MAUI application, optionally restoring previously persisted state.

## Signatures

```csharp
void static CreatePlatformWindow(this Android.App.Activity! activity, Microsoft.Maui.IApplication! application, Android.OS.Bundle? savedInstanceState = null)
void static CreatePlatformWindow(this UIKit.IUIApplicationDelegate! platformApplication, Microsoft.Maui.IApplication! application, UIKit.UIApplication! uiApplication, Foundation.NSDictionary? launchOptions)
void static CreatePlatformWindow(this UIKit.IUIWindowSceneDelegate! sceneDelegate, Microsoft.Maui.IApplication! application, UIKit.UIScene! scene, UIKit.UISceneSession! session, UIKit.UISceneConnectionOptions! connectionOptions)
void static CreatePlatformWindow(this Microsoft.UI.Xaml.Application! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.Handlers.OpenWindowRequest? args)
void static CreatePlatformWindow(this Microsoft.UI.Xaml.Application! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.UI.Xaml.LaunchActivatedEventArgs? args)
```

## See also

- Declaring type: [[ApplicationExtensions (Microsoft.Maui.Platform)|ApplicationExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
