---
title: "BasePlatformPermission.RequiredPrivileges"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePlatformPermission.RequiredPrivileges"
declaring_type: "BasePlatformPermission"
member_kind: property
---

# BasePlatformPermission.RequiredPrivileges

> [!abstract] Property of [[BasePlatformPermission|BasePlatformPermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Gets the Tizen privileges required by this permission, along with whether each is a runtime privilege.

## Signature

```csharp
virtual (string tizenPrivilege, bool isRuntime)[] RequiredPrivileges { get; }
```

## See also

- Declaring type: [[BasePlatformPermission|BasePlatformPermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
