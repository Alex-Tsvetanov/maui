---
title: "IFontManager.GetFontSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IFontManager.GetFontSize"
declaring_type: "IFontManager"
member_kind: method
---

# IFontManager.GetFontSize

> [!abstract] Method of [[IFontManager|IFontManager]]
> Namespace: `Microsoft.Maui`

Gets the font size for the provided font.

## Signatures

```csharp
Microsoft.Maui.FontSize GetFontSize(Microsoft.Maui.Font font, float defaultFontSize = 0)
double GetFontSize(Microsoft.Maui.Font font, double defaultFontSize = 0)
```

## Remarks

If `AutoScalingEnabled` is `true` on `font`, the returned `FontSize` is expressed in `Sp`, otherwise `Dip`.

## Returns

If `Size` is more than 0 and no equal to `NaN`, returns `Size`. Else, if `defaultFontSize` is more than 0, returns `defaultFontSize`. Else, returns `DefaultFontSize`.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The font to get the size for. |
| `defaultFontSize` | Default font size when the provided font does not have a (valid) value. |

## See also

- Declaring type: [[IFontManager|IFontManager]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
