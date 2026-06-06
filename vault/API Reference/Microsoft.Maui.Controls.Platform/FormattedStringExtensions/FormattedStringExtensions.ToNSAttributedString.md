---
title: "FormattedStringExtensions.ToNSAttributedString"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.FormattedStringExtensions.ToNSAttributedString"
declaring_type: "FormattedStringExtensions"
member_kind: method
---

# FormattedStringExtensions.ToNSAttributedString

> [!abstract] Method of [[FormattedStringExtensions|FormattedStringExtensions]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Converts the MAUI formatted string to an iOS NSAttributedString using the supplied font manager and default formatting.

## Signatures

```csharp
Foundation.NSAttributedString! static ToNSAttributedString(this Microsoft.Maui.Controls.FormattedString! formattedString, Microsoft.Maui.IFontManager! fontManager, double defaultLineHeight = -1, Microsoft.Maui.TextAlignment defaultHorizontalAlignment = Microsoft.Maui.TextAlignment.Start, Microsoft.Maui.Font? defaultFont = null, Microsoft.Maui.Graphics.Color? defaultColor = null, Microsoft.Maui.TextTransform defaultTextTransform = Microsoft.Maui.TextTransform.Default)
Foundation.NSAttributedString? static ToNSAttributedString(this Microsoft.Maui.Controls.Label! label)
Foundation.NSAttributedString! static ToNSAttributedString(this Microsoft.Maui.Controls.Span! span, Microsoft.Maui.IFontManager! fontManager, double defaultLineHeight = -1, Microsoft.Maui.TextAlignment defaultHorizontalAlignment = Microsoft.Maui.TextAlignment.Start, Microsoft.Maui.Font? defaultFont = null, Microsoft.Maui.Graphics.Color? defaultColor = null, Microsoft.Maui.TextTransform defaultTextTransform = Microsoft.Maui.TextTransform.Default)
```

## See also

- Declaring type: [[FormattedStringExtensions|FormattedStringExtensions]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
