---
title: "Locale.Language"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.Locale.Language"
declaring_type: "Locale"
member_kind: property
---

# Locale.Language

> [!abstract] Property of [[Locale|Locale]]
> Namespace: `Microsoft.Maui.Media`

Gets the language name or code.

## Signature

```csharp
string! Language { get; }
```

## Remarks

This value may vary between platforms. For Android this used the ISO 639 alpha-2 or alpha-3 language code, or registered language subtags up to 8 alpha letters (for future enhancements). When a language has both an alpha-2 code and an alpha-3 code, the alpha-2 code must be used. For iOS and Windows this uses the BCP-47 language code.

## See also

- Declaring type: [[Locale|Locale]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
