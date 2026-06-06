---
title: "PictureReaderExtensions.Read"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PictureReaderExtensions.Read"
declaring_type: "PictureReaderExtensions"
member_kind: method
---

# PictureReaderExtensions.Read

> [!abstract] Method of [[PictureReaderExtensions|PictureReaderExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Reads a picture from a stream.

## Signature

```csharp
Microsoft.Maui.Graphics.IPicture static Read(this Microsoft.Maui.Graphics.IPictureReader target, System.IO.Stream stream, string hash = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `target` | The picture reader. |
| `stream` | The stream containing the picture data. |
| `hash` | Optional hash value for the picture data. |

## Returns

An `IPicture` object read from the stream.

## See also

- Declaring type: [[PictureReaderExtensions|PictureReaderExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
