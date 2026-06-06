---
title: "ConnectivityChangedEventArgs.NetworkAccess"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Networking
aliases:
  - "Microsoft.Maui.Networking.ConnectivityChangedEventArgs.NetworkAccess"
declaring_type: "ConnectivityChangedEventArgs"
member_kind: property
---

# ConnectivityChangedEventArgs.NetworkAccess

> [!abstract] Property of [[ConnectivityChangedEventArgs|ConnectivityChangedEventArgs]]
> Namespace: `Microsoft.Maui.Networking`

Gets the current state of network access.

## Signature

```csharp
Microsoft.Maui.Networking.NetworkAccess NetworkAccess { get; }
```

## Remarks

Even when `Internet` is returned, full internet access is not guaranteed. Can throw `PermissionException` on Android if ACCESS_NETWORK_STATE is not set in manifest.

## See also

- Declaring type: [[ConnectivityChangedEventArgs|ConnectivityChangedEventArgs]]
- [[_Microsoft.Maui.Networking|Microsoft.Maui.Networking namespace]]
