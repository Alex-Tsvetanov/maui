---
title: "FontManager.GetFontFamily"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontManager.GetFontFamily"
declaring_type: "FontManager"
member_kind: method
---

# FontManager.GetFontFamily

> [!abstract] Method of [[FontManager|FontManager]]
> Namespace: `Microsoft.Maui`

Retrieves the platform equivalent string with which a font can be applied for a specified font family.

## Signatures

```csharp
string! GetFontFamily(string? fontFamily)
Microsoft.UI.Xaml.Media.FontFamily! GetFontFamily(Microsoft.Maui.Font font)
```

## Returns

A `string` containing styling information to apply the font.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The font family to get the platform equivalent for. |

## See also

- Declaring type: [[FontManager|FontManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
