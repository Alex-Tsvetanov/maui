---
title: "FontRegistrar.GetFont"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontRegistrar.GetFont"
declaring_type: "FontRegistrar"
member_kind: method
---

# FontRegistrar.GetFont

> [!abstract] Method of [[FontRegistrar|FontRegistrar]]
> Namespace: `Microsoft.Maui`

Retrieves the platform equivalent `UIFont` for an abstract `Font` object.

## Signature

```csharp
string? GetFont(string! font)
```

## Returns

The `UIFont` object representing the font as provided in `font`.

## Parameters

| Parameter | Description |
|---|---|
| `font` | The abstract font representation to get the platform equivalent for. |
| `defaultFontSize` | The default font size to use for this font if no size is specified in `font`. |

## See also

- Declaring type: [[FontRegistrar|FontRegistrar]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
