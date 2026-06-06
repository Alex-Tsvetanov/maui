---
title: "IMediaPicker.CapturePhotoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.IMediaPicker.CapturePhotoAsync"
declaring_type: "IMediaPicker"
member_kind: method
---

# IMediaPicker.CapturePhotoAsync

> [!abstract] Method of [[IMediaPicker|IMediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the camera to take a photo.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! CapturePhotoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Returns

A `FileResult` object containing details of the captured photo. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[IMediaPicker|IMediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
