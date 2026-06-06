---
title: "PictureWriterExtensions.SaveAsBytes"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PictureWriterExtensions.SaveAsBytes"
declaring_type: "PictureWriterExtensions"
member_kind: method
---

# PictureWriterExtensions.SaveAsBytes

> [!abstract] Method of [[PictureWriterExtensions|PictureWriterExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Saves a picture as a byte array.

## Signature

```csharp
byte[] static SaveAsBytes(this Microsoft.Maui.Graphics.IPictureWriter target, Microsoft.Maui.Graphics.IPicture picture)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The picture writer to use for saving. |
| `picture` | The picture to save. |

## Returns

A byte array containing the saved picture data, or null if either the target or picture is null.

## See also

- Declaring type: [[PictureWriterExtensions|PictureWriterExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
