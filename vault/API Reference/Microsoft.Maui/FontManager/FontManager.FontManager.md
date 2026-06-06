---
title: "FontManager.FontManager"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontManager.FontManager"
declaring_type: "FontManager"
member_kind: constructor
---

# FontManager.FontManager

> [!abstract] Constructor of [[FontManager|FontManager]]
> Namespace: `Microsoft.Maui`

Creates a new `EmbeddedFontLoader` instance.

## Signature

```csharp
void FontManager(Microsoft.Maui.IFontRegistrar! fontRegistrar, System.IServiceProvider? serviceProvider = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `fontRegistrar` | A `IFontRegistrar` instance to retrieve details from about registered fonts. |
| `serviceProvider` | The applications `IServiceProvider`. Typically this is provided through dependency injection. |

## See also

- Declaring type: [[FontManager|FontManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
