---
title: "Connectivity.ConnectivityChanged"
tags:
  - api
  - member/event
  - ns/Microsoft-Maui-Networking
aliases:
  - "Microsoft.Maui.Networking.Connectivity.ConnectivityChanged"
declaring_type: "Connectivity"
member_kind: event
---

# Connectivity.ConnectivityChanged

> [!abstract] Event of [[Connectivity|Connectivity]]
> Namespace: `Microsoft.Maui.Networking`

Occurs when network access or profile has changed.

## Signature

```csharp
System.EventHandler<Microsoft.Maui.Networking.ConnectivityChangedEventArgs!>! static ConnectivityChanged
```

## Remarks

Can throw `PermissionException` on Android if ACCESS_NETWORK_STATE is not set in manifest.

## See also

- Declaring type: [[Connectivity|Connectivity]]
- [[_Microsoft.Maui.Networking|Microsoft.Maui.Networking namespace]]
