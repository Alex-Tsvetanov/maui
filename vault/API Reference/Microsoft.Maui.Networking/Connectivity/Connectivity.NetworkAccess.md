---
title: "Connectivity.NetworkAccess"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Networking
aliases:
  - "Microsoft.Maui.Networking.Connectivity.NetworkAccess"
declaring_type: "Connectivity"
member_kind: property
---

# Connectivity.NetworkAccess

> [!abstract] Property of [[Connectivity|Connectivity]]
> Namespace: `Microsoft.Maui.Networking`

Gets the current state of network access.

## Signature

```csharp
static Microsoft.Maui.Networking.NetworkAccess NetworkAccess { get; }
```

## Remarks

Even when `Internet` is returned, full internet access is not guaranteed. Can throw `PermissionException` on Android if ACCESS_NETWORK_STATE is not set in manifest.

## See also

- Declaring type: [[Connectivity|Connectivity]]
- [[_Microsoft.Maui.Networking|Microsoft.Maui.Networking namespace]]
