---
title: "FormattedStringExtensions.ToSpannableString"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.FormattedStringExtensions.ToSpannableString"
declaring_type: "FormattedStringExtensions"
member_kind: method
---

# FormattedStringExtensions.ToSpannableString

> [!abstract] Method of [[FormattedStringExtensions|FormattedStringExtensions]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Converts the MAUI formatted string to an Android SpannableString using the supplied font manager and default formatting.

## Signatures

```csharp
Android.Text.SpannableString! static ToSpannableString(this Microsoft.Maui.Controls.FormattedString! formattedString, Microsoft.Maui.IFontManager! fontManager, Android.Content.Context? context = null, double defaultCharacterSpacing = 0, Microsoft.Maui.TextAlignment defaultHorizontalAlignment = Microsoft.Maui.TextAlignment.Start, Microsoft.Maui.Font? defaultFont = null, Microsoft.Maui.Graphics.Color? defaultColor = null, Microsoft.Maui.TextTransform defaultTextTransform = Microsoft.Maui.TextTransform.Default, Microsoft.Maui.TextDecorations defaultTextDecorations = Microsoft.Maui.TextDecorations.None)
Android.Text.SpannableString! static ToSpannableString(this Microsoft.Maui.Controls.Label! label)
```

## See also

- Declaring type: [[FormattedStringExtensions|FormattedStringExtensions]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
