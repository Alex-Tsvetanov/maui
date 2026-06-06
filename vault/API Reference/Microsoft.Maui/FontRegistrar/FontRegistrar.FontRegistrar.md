---
title: "FontRegistrar.FontRegistrar"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontRegistrar.FontRegistrar"
declaring_type: "FontRegistrar"
member_kind: constructor
---

# FontRegistrar.FontRegistrar

> [!abstract] Constructor of [[FontRegistrar|FontRegistrar]]
> Namespace: `Microsoft.Maui`

Creates a new instance of `FontRegistrar`.

## Signature

```csharp
void FontRegistrar(Microsoft.Maui.IEmbeddedFontLoader! fontLoader, System.IServiceProvider? serviceProvider = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `fontLoader` | An instance of `IEmbeddedFontLoader` that is responsible for actually loading fonts. |
| `serviceProvider` | A reference to the app `IServiceProvider`. Typically this should be provided through dependency injection for logging purposes, otherwise can be ignored. |

## See also

- Declaring type: [[FontRegistrar|FontRegistrar]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
