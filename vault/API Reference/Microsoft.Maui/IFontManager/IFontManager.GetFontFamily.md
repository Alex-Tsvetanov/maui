---
title: "IFontManager.GetFontFamily"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IFontManager.GetFontFamily"
declaring_type: "IFontManager"
member_kind: method
---

# IFontManager.GetFontFamily

> [!abstract] Method of [[IFontManager|IFontManager]]
> Namespace: `Microsoft.Maui`

Retrieves the platform equivalent string with which a font can be applied for a specified font family.

## Signatures

```csharp
string! GetFontFamily(string? font)
Microsoft.UI.Xaml.Media.FontFamily! GetFontFamily(Microsoft.Maui.Font font)
```

## Returns

A `string` containing styling information to apply the font.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The font family to get the platform equivalent for. |

## See also

- Declaring type: [[IFontManager|IFontManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
