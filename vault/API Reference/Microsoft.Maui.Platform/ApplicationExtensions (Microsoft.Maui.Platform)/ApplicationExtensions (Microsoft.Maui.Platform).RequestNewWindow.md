---
title: "ApplicationExtensions (Microsoft.Maui.Platform).RequestNewWindow"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.ApplicationExtensions.RequestNewWindow"
declaring_type: "ApplicationExtensions (Microsoft.Maui.Platform)"
member_kind: method
---

# ApplicationExtensions (Microsoft.Maui.Platform).RequestNewWindow

> [!abstract] Method of [[ApplicationExtensions (Microsoft.Maui.Platform)|ApplicationExtensions (Microsoft.Maui.Platform)]]
> Namespace: `Microsoft.Maui.Platform`

Requests that the platform application open a new window for the specified MAUI application using the given open-window arguments.

## Signatures

```csharp
void static RequestNewWindow(this Android.App.Application! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.Handlers.OpenWindowRequest? args)
void static RequestNewWindow(this UIKit.IUIApplicationDelegate! platformApplication, Microsoft.Maui.IApplication! application, Microsoft.Maui.Handlers.OpenWindowRequest? args)
```

## See also

- Declaring type: [[ApplicationExtensions (Microsoft.Maui.Platform)|ApplicationExtensions (Microsoft.Maui.Platform)]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
