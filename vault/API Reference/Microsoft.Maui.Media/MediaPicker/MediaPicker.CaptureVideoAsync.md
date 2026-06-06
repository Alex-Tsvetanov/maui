---
title: "MediaPicker.CaptureVideoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPicker.CaptureVideoAsync"
declaring_type: "MediaPicker"
member_kind: method
---

# MediaPicker.CaptureVideoAsync

> [!abstract] Method of [[MediaPicker|MediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the camera to take a video.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! static CaptureVideoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Returns

A `FileResult` object containing details of the captured video. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[MediaPicker|MediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
