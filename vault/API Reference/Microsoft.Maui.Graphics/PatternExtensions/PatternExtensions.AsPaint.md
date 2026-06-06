---
title: "PatternExtensions.AsPaint"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PatternExtensions.AsPaint"
declaring_type: "PatternExtensions"
member_kind: method
---

# PatternExtensions.AsPaint

> [!abstract] Method of [[PatternExtensions|PatternExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Converts a pattern to a paint using black as the foreground color.

## Signatures

```csharp
Microsoft.Maui.Graphics.Paint static AsPaint(this Microsoft.Maui.Graphics.IPattern target, Microsoft.Maui.Graphics.Color foregroundColor)
Microsoft.Maui.Graphics.Paint static AsPaint(this Microsoft.Maui.Graphics.IPattern target)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The pattern to convert. |

## Returns

A `Paint` configured with the pattern and black foreground color, or null if the pattern is null.

## See also

- Declaring type: [[PatternExtensions|PatternExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
