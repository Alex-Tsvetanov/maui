---
title: "IPlatformFonts.Get"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.IPlatformFonts.Get"
declaring_type: "IPlatformFonts"
member_kind: method
---

# IPlatformFonts.Get

> [!abstract] Method of [[IPlatformFonts|IPlatformFonts]]
> Namespace: `Microsoft.Maui.Graphics`

Gets the platform-specific font object for the specified font.

## Signatures

```csharp
object! Get(Microsoft.Maui.Graphics.IFont! font)
object! Get(string! alias, int weight = 400, Microsoft.Maui.Graphics.FontStyleType fontStyleType = Microsoft.Maui.Graphics.FontStyleType.Normal)
```

## Returns

A platform-specific font object.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The font to get the platform-specific object for. |

## See also

- Declaring type: [[IPlatformFonts|IPlatformFonts]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
