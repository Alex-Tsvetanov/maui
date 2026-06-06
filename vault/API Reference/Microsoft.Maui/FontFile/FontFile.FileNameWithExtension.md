---
title: "FontFile.FileNameWithExtension"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.FontFile.FileNameWithExtension"
declaring_type: "FontFile"
member_kind: method
---

# FontFile.FileNameWithExtension

> [!abstract] Method of [[FontFile|FontFile]]
> Namespace: `Microsoft.Maui`

Gets the filename of this font file with the provided extension appended to the end.

## Signatures

```csharp
string! FileNameWithExtension()
string! FileNameWithExtension(string? extension)
```

## Parameters

| Parameter | Description |
|---|---|
| `extension` | The extension to append to the font filename. |

## Returns

The filename of this font file including the given extension.

## Remarks

The value for `extension` should include a leading dot (.) character.

## See also

- Declaring type: [[FontFile|FontFile]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
