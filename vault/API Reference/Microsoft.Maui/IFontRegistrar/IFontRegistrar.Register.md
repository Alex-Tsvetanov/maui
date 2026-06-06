---
title: "IFontRegistrar.Register"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.IFontRegistrar.Register"
declaring_type: "IFontRegistrar"
member_kind: method
---

# IFontRegistrar.Register

> [!abstract] Method of [[IFontRegistrar|IFontRegistrar]]
> Namespace: `Microsoft.Maui`

Registers a font in the app font cache.

## Signatures

```csharp
void Register(string! filename, string? alias, System.Reflection.Assembly! assembly)
void Register(string! filename, string? alias)
```

## Parameters

| Parameter | Description |
|---|---|
| `filename` | The filename of the font to register. |
| `alias` | An optional alias with which you can also refer to this font. |
| `assembly` | The assembly (not used). |

## See also

- Declaring type: [[IFontRegistrar|IFontRegistrar]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
