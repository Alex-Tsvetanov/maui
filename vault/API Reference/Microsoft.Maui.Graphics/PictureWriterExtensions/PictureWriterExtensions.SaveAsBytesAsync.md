---
title: "PictureWriterExtensions.SaveAsBytesAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PictureWriterExtensions.SaveAsBytesAsync"
declaring_type: "PictureWriterExtensions"
member_kind: method
---

# PictureWriterExtensions.SaveAsBytesAsync

> [!abstract] Method of [[PictureWriterExtensions|PictureWriterExtensions]]
> Namespace: `Microsoft.Maui.Graphics`

Asynchronously saves a picture as a byte array.

## Signature

```csharp
System.Threading.Tasks.Task<byte[]> static SaveAsBytesAsync(this Microsoft.Maui.Graphics.IPictureWriter target, Microsoft.Maui.Graphics.IPicture picture)
```

## Returns

A task that represents the asynchronous save operation. The task result contains the saved picture data as a byte array, or null if either the target or picture is null.

## Parameters

| Parameter | Description |
|---|---|
| `target` | The picture writer to use for saving. |
| `picture` | The picture to save. |

## See also

- Declaring type: [[PictureWriterExtensions|PictureWriterExtensions]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
