---
title: "Locale.Country"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.Locale.Country"
declaring_type: "Locale"
member_kind: property
---

# Locale.Country

> [!abstract] Property of [[Locale|Locale]]
> Namespace: `Microsoft.Maui.Media`

Gets the country name or code.

## Signature

```csharp
string! Country { get; }
```

## Remarks

This value may vary between platforms. For Android this used the ISO 3166 alpha-2 country code or UN M.49 numeric-3 area code. For iOS and Windows this field is not used and `null` .

## See also

- Declaring type: [[Locale|Locale]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
