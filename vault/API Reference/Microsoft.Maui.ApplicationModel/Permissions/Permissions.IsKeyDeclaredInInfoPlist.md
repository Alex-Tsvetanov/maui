---
title: "Permissions.IsKeyDeclaredInInfoPlist"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.IsKeyDeclaredInInfoPlist"
declaring_type: "Permissions"
member_kind: method
---

# Permissions.IsKeyDeclaredInInfoPlist

> [!abstract] Method of [[Permissions|Permissions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Checks if the key specified in `usageKey` is declared in the application's Info.plist file.

## Signature

```csharp
bool static IsKeyDeclaredInInfoPlist(string usageKey)
```

## Parameters

| Parameter | Description |
|---|---|
| `usageKey` | The key to check for declaration in the Info.plist file. |

## Returns

`true` when the key is declared, otherwise `false`.

## See also

- Declaring type: [[Permissions|Permissions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
