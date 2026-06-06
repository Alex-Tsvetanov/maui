---
title: "BasePermission.ShouldShowRationale"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePermission.ShouldShowRationale"
declaring_type: "BasePermission"
member_kind: method
---

# BasePermission.ShouldShowRationale

> [!abstract] Method of [[BasePermission|BasePermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Determines if an educational UI should be displayed explaining to the user how this permission will be used in the application.

## Signature

```csharp
bool abstract ShouldShowRationale()
```

## Returns

`true` if the user has denied or disabled this permission in the past, else `false`.

## Remarks

Only used on Android, other platforms will always return `false`.

## See also

- Declaring type: [[BasePermission|BasePermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
