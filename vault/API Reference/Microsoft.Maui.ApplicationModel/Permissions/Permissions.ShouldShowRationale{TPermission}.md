---
title: "Permissions.ShouldShowRationale<TPermission>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.ShouldShowRationale<TPermission>"
declaring_type: "Permissions"
member_kind: method
---

# Permissions.ShouldShowRationale<TPermission>

> [!abstract] Method of [[Permissions|Permissions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Determines if an educational UI should be displayed explaining to the user how the permission will be used in the application.

## Signature

```csharp
bool static ShouldShowRationale<TPermission>()
```

## Remarks

Only used on Android, other platforms will always return `false`.

## Returns

`true` if the user has denied or disabled the permission in the past, else `false`.

## See also

- Declaring type: [[Permissions|Permissions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
