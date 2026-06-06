---
title: "Permissions.IsPrivilegeDeclared"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.Permissions.IsPrivilegeDeclared"
declaring_type: "Permissions"
member_kind: method
---

# Permissions.IsPrivilegeDeclared

> [!abstract] Method of [[Permissions|Permissions]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Checks if the key specified in `tizenPrivilege` is declared in the application's tizen-manifest.xml file.

## Signature

```csharp
bool static IsPrivilegeDeclared(string tizenPrivilege)
```

## Parameters

| Parameter | Description |
|---|---|
| `tizenPrivilege` | The key to check for declaration in the tizen-manifest.xml file. |

## Returns

`true` when the key is declared, otherwise `false`.

## See also

- Declaring type: [[Permissions|Permissions]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
