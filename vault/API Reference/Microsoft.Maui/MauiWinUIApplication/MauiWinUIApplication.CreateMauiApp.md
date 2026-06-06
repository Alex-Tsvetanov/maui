---
title: "MauiWinUIApplication.CreateMauiApp"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.MauiWinUIApplication.CreateMauiApp"
declaring_type: "MauiWinUIApplication"
member_kind: method
---

# MauiWinUIApplication.CreateMauiApp

> [!abstract] Method of [[MauiWinUIApplication|MauiWinUIApplication]]
> Namespace: `Microsoft.Maui`

When overridden in a derived class, creates the `MauiApp` to be used in this application. Typically a `MauiApp` is created by calling `CreateBuilder`, configuring the returned `MauiAppBuilder`, and returning the built app by calling `Build`.

## Signature

```csharp
Microsoft.Maui.Hosting.MauiApp! abstract CreateMauiApp()
```

## Returns

The built `MauiApp`.

## See also

- Declaring type: [[MauiWinUIApplication|MauiWinUIApplication]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
