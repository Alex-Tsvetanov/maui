---
title: "BasePermission.EnsureDeclared"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePermission.EnsureDeclared"
declaring_type: "BasePermission"
member_kind: method
---

# BasePermission.EnsureDeclared

> [!abstract] Method of [[BasePermission|BasePermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Ensures that a required entry matching this permission is found in the application manifest file.

## Signature

```csharp
void abstract EnsureDeclared()
```

## Remarks

Will throw `PermissionException` if a required entry was not found in the application manifest. Not all permissions require a manifest entry.

## See also

- Declaring type: [[BasePermission|BasePermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
