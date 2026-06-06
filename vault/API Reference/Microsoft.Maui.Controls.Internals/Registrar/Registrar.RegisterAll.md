---
title: "Registrar.RegisterAll"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.Registrar.RegisterAll"
declaring_type: "Registrar"
member_kind: method
---

# Registrar.RegisterAll

> [!abstract] Method of [[Registrar|Registrar]]
> Namespace: `Microsoft.Maui.Controls.Internals`

Registers all specified attribute types with an optional font registrar.

## Signatures

```csharp
void static RegisterAll(System.Type[] attrTypes, Microsoft.Maui.Controls.InitializationFlags flags, Microsoft.Maui.IFontRegistrar fontRegistrar = null)
void static RegisterAll(System.Type[] attrTypes, Microsoft.Maui.IFontRegistrar fontRegistrar = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `attrTypes` | Array of attribute types to register. |
| `fontRegistrar` | Optional font registrar for handling font registration during the process. |

## Remarks

For internal use only. This API can be changed or removed without notice at any time.

## See also

- Declaring type: [[Registrar|Registrar]]
- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
