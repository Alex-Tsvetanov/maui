---
title: "BasePlatformPermission.EnsureDeclared"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePlatformPermission.EnsureDeclared"
declaring_type: "BasePlatformPermission"
member_kind: method
---

# BasePlatformPermission.EnsureDeclared

> [!abstract] Method of [[BasePlatformPermission|BasePlatformPermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Ensures that a required entry matching this permission is found in the application manifest file.

## Signature

```csharp
void override EnsureDeclared()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## See also

- Declaring type: [[BasePlatformPermission|BasePlatformPermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
