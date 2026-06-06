---
title: "BasePlatformPermission.ShouldShowRationale"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.BasePlatformPermission.ShouldShowRationale"
declaring_type: "BasePlatformPermission"
member_kind: method
---

# BasePlatformPermission.ShouldShowRationale

> [!abstract] Method of [[BasePlatformPermission|BasePlatformPermission]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Determines if an educational UI should be displayed explaining to the user how the permission will be used in the application.

## Signature

```csharp
bool override ShouldShowRationale()
```

## Remarks

Only used on Android, other platforms will always return `false`.

## Returns

`true` if the user has denied or disabled the permission in the past, else `false`.

## See also

- Declaring type: [[BasePlatformPermission|BasePlatformPermission]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
