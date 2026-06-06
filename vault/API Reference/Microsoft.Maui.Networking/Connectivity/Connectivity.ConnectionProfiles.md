---
title: "Connectivity.ConnectionProfiles"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Networking
aliases:
  - "Microsoft.Maui.Networking.Connectivity.ConnectionProfiles"
declaring_type: "Connectivity"
member_kind: property
---

# Connectivity.ConnectionProfiles

> [!abstract] Property of [[Connectivity|Connectivity]]
> Namespace: `Microsoft.Maui.Networking`

Gets the active connectivity types for the device.

## Signature

```csharp
static System.Collections.Generic.IEnumerable<Microsoft.Maui.Networking.ConnectionProfile>! ConnectionProfiles { get; }
```

## Remarks

Can throw `PermissionException` on Android if ACCESS_NETWORK_STATE is not set in manifest.

## See also

- Declaring type: [[Connectivity|Connectivity]]
- [[_Microsoft.Maui.Networking|Microsoft.Maui.Networking namespace]]
