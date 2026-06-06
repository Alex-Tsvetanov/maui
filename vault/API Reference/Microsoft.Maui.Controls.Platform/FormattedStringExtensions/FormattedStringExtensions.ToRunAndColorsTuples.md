---
title: "FormattedStringExtensions.ToRunAndColorsTuples"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.FormattedStringExtensions.ToRunAndColorsTuples"
declaring_type: "FormattedStringExtensions"
member_kind: method
---

# FormattedStringExtensions.ToRunAndColorsTuples

> [!abstract] Method of [[FormattedStringExtensions|FormattedStringExtensions]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Converts the formatted string's spans to a sequence of tuples each containing a WinUI text run and its foreground and background colors.

## Signature

```csharp
System.Collections.Generic.IEnumerable<System.Tuple<Microsoft.UI.Xaml.Documents.Run!, Microsoft.Maui.Graphics.Color!, Microsoft.Maui.Graphics.Color!>!>! static ToRunAndColorsTuples(this Microsoft.Maui.Controls.FormattedString! formattedString, Microsoft.Maui.IFontManager! fontManager, double defaultLineHeight = -1, Microsoft.Maui.TextAlignment defaultHorizontalAlignment = Microsoft.Maui.TextAlignment.Start, Microsoft.Maui.Font? defaultFont = null, Microsoft.Maui.Graphics.Color? defaultColor = null, Microsoft.Maui.TextTransform defaultTextTransform = Microsoft.Maui.TextTransform.Default)
```

## See also

- Declaring type: [[FormattedStringExtensions|FormattedStringExtensions]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
