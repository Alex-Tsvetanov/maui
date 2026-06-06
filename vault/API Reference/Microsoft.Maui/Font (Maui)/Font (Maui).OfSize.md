---
title: "Font (Maui).OfSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.Font.OfSize"
declaring_type: "Font (Maui)"
member_kind: method
---

# Font (Maui).OfSize

> [!abstract] Method of [[Font (Maui)|Font (Maui)]]
> Namespace: `Microsoft.Maui`

Creates a font with the specified family, size, weight, slant, and auto-scaling.

## Signature

```csharp
Microsoft.Maui.Font static OfSize(string? name, double size, Microsoft.Maui.FontWeight weight = Microsoft.Maui.FontWeight.Regular, Microsoft.Maui.FontSlant fontSlant = Microsoft.Maui.FontSlant.Default, bool enableScaling = true)
```

## Parameters

| Parameter | Description |
|---|---|
| `name` | The font family name or system font alias. |
| `size` | The desired font size. |
| `weight` | The font weight. |
| `fontSlant` | The font slant. |
| `enableScaling` | Whether auto-scaling is enabled. |

## Returns

A `Font` instance with the specified settings.

## See also

- Declaring type: [[Font (Maui)|Font (Maui)]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
