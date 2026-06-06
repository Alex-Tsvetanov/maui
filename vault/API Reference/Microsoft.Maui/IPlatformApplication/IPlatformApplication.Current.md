---
title: "IPlatformApplication.Current"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IPlatformApplication.Current"
declaring_type: "IPlatformApplication"
member_kind: property
---

# IPlatformApplication.Current

> [!abstract] Property of [[IPlatformApplication|IPlatformApplication]]
> Namespace: `Microsoft.Maui`

Gets or sets the current platform application instance.

## Signature

```csharp
Microsoft.Maui.IPlatformApplication? Current { get; set; }
```

## Remarks

This property provides access to the platform-specific application instance and its services. It must be manually set by each platform implementation during application startup. Common usage scenarios: Accessing platform services: IPlatformApplication.Current?.Services Getting the application instance: IPlatformApplication.Current?.Application Platform-specific operations requiring the native application context Always check for `null` before using this property, especially during application startup or in unit tests where the platform application may not be initialized.

## See also

- Declaring type: [[IPlatformApplication|IPlatformApplication]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
