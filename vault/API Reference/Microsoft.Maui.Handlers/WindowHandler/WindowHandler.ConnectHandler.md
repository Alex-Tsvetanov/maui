---
title: "WindowHandler.ConnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.WindowHandler.ConnectHandler"
declaring_type: "WindowHandler"
member_kind: method
---

# WindowHandler.ConnectHandler

> [!abstract] Method of [[WindowHandler|WindowHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Connects the handler to the native window (Android Activity, iOS UIWindow, or WinUI Window) backing the cross-platform window.

## Signatures

```csharp
void override ConnectHandler(Android.App.Activity! platformView)
void override ConnectHandler(UIKit.UIWindow! platformView)
void override ConnectHandler(Microsoft.UI.Xaml.Window! platformView)
```

## See also

- Declaring type: [[WindowHandler|WindowHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
