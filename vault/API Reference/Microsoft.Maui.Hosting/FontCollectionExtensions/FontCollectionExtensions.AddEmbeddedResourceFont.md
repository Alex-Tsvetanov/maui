---
title: "FontCollectionExtensions.AddEmbeddedResourceFont"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Hosting
aliases:
  - "Microsoft.Maui.Hosting.FontCollectionExtensions.AddEmbeddedResourceFont"
declaring_type: "FontCollectionExtensions"
member_kind: method
---

# FontCollectionExtensions.AddEmbeddedResourceFont

> [!abstract] Method of [[FontCollectionExtensions|FontCollectionExtensions]]
> Namespace: `Microsoft.Maui.Hosting`

Adds the font specified in `filename` from an embedded resource in `assembly` to the `fontCollection`, with an optional font alias specified in `alias`.

## Signature

```csharp
Microsoft.Maui.Hosting.IFontCollection! static AddEmbeddedResourceFont(this Microsoft.Maui.Hosting.IFontCollection! fontCollection, System.Reflection.Assembly! assembly, string! filename, string? alias = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `fontCollection` | The collection to add the font to. |
| `assembly` | The assembly that contains the specified font as an embedded resource. |
| `filename` | The embedded resource filename of the font to add, such as a True type format (TTF) or open type font (OTF) font file. |
| `alias` | An optional alias that can also be used to reference this font. |

## See also

- Declaring type: [[FontCollectionExtensions|FontCollectionExtensions]]
- [[_Microsoft.Maui.Hosting|Microsoft.Maui.Hosting namespace]]
