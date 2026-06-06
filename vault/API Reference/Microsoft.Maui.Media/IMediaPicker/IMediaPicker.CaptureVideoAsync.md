---
title: "IMediaPicker.CaptureVideoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.IMediaPicker.CaptureVideoAsync"
declaring_type: "IMediaPicker"
member_kind: method
---

# IMediaPicker.CaptureVideoAsync

> [!abstract] Method of [[IMediaPicker|IMediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the camera to take a video.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! CaptureVideoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Returns

A `FileResult` object containing details of the captured video. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[IMediaPicker|IMediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
