---
title: "FontManager.GetFont"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontManager.GetFont"
declaring_type: "FontManager"
member_kind: method
---

# FontManager.GetFont

> [!abstract] Method of [[FontManager|FontManager]]
> Namespace: `Microsoft.Maui`

Retrieves the platform equivalent `UIFont` for an abstract `Font` object.

## Signatures

```csharp
UIKit.UIFont! GetFont(Microsoft.Maui.Font font, double defaultFontSize = 0)
string! GetFont(Microsoft.Maui.Font font)
```

## Returns

The `UIFont` object representing the font as provided in `font`.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The abstract font representation to get the platform equivalent for. |
| `defaultFontSize` | The default font size to use for this font if no size is specified in `font`. |

## See also

- Declaring type: [[FontManager|FontManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
