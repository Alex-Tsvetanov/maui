---
title: "PictureReaderExtensions.ReadAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PictureReaderExtensions.ReadAsync"
declaring_type: "PictureReaderExtensions"
member_kind: method
---

# PictureReaderExtensions.ReadAsync

> [!abstract] Method of [[PictureReaderExtensions|PictureReaderExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Asynchronously reads a picture from a stream.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Graphics.IPicture> static ReadAsync(this Microsoft.Maui.Graphics.IPictureReader target, System.IO.Stream stream, string hash = null)
```

## Returns

A task that represents the asynchronous read operation. The result contains an `IPicture` object read from the stream.

## Parameters

| Parameter | Description |
|---|---|
| `target` | The picture reader. |
| `stream` | The stream containing the picture data. |
| `hash` | Optional hash value for the picture data. |

## See also

- Declaring type: [[PictureReaderExtensions|PictureReaderExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
