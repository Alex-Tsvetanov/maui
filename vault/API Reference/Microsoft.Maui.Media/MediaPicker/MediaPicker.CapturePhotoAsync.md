---
title: "MediaPicker.CapturePhotoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPicker.CapturePhotoAsync"
declaring_type: "MediaPicker"
member_kind: method
---

# MediaPicker.CapturePhotoAsync

> [!abstract] Method of [[MediaPicker|MediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the camera to take a photo.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! static CapturePhotoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Returns

A `FileResult` object containing details of the captured photo. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[MediaPicker|MediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
