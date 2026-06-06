---
title: "FontFile.PostScriptName"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontFile.PostScriptName"
declaring_type: "FontFile"
member_kind: property
---

# FontFile.PostScriptName

> [!abstract] Property of [[FontFile|FontFile]]
> Namespace: `Microsoft.Maui`

The font PostScript name as read from the font file.

## Signature

```csharp
string? PostScriptName { get; set; }
```

## Remarks

Some platforms have issues with spaces in the PostScript name. In this property spaces might be stripped. To get the PostScript name with spaces (if it had any) use `GetPostScriptNameWithSpaces`.

## See also

- Declaring type: [[FontFile|FontFile]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
