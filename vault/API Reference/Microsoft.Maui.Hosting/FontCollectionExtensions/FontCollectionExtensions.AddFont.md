---
title: "FontCollectionExtensions.AddFont"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.FontCollectionExtensions.AddFont"
declaring_type: "FontCollectionExtensions"
member_kind: method
---

# FontCollectionExtensions.AddFont

> [!abstract] Method of [[FontCollectionExtensions|FontCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Adds the font specified in `filename` to the `fontCollection`, with an optional font alias specified in `alias`.

## Signature

```csharp
Microsoft.Maui.Hosting.IFontCollection! static AddFont(this Microsoft.Maui.Hosting.IFontCollection! fontCollection, string! filename, string? alias = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `fontCollection` | The collection to add the font to. |
| `filename` | The filename of the font to add, such as a True type format (TTF) or open type font (OTF) font file. Font files can be added to the 'Resources\Fonts' of a .NET MAUI project. |
| `alias` | An optional alias that can also be used to reference this font. |

## See also

- Declaring type: [[FontCollectionExtensions|FontCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
