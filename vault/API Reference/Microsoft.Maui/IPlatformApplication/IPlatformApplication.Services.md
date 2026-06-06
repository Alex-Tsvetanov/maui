---
title: "IPlatformApplication.Services"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IPlatformApplication.Services"
declaring_type: "IPlatformApplication"
member_kind: property
---

# IPlatformApplication.Services

> [!abstract] Property of [[IPlatformApplication|IPlatformApplication]]
> Namespace: `Microsoft.Maui`

Gets the dependency injection service provider for the platform application.

## Signature

```csharp
System.IServiceProvider! Services { get; }
```

## Remarks

Use this service provider to resolve services that have been registered with the platform's dependency injection container. This includes both framework services and custom services registered during application configuration.

## See also

- Declaring type: [[IPlatformApplication|IPlatformApplication]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
