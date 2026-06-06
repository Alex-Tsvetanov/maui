---
title: "MediaPicker.PickVideosAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPicker.PickVideosAsync"
declaring_type: "MediaPicker"
member_kind: method
---

# MediaPicker.PickVideosAsync

> [!abstract] Method of [[MediaPicker|MediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the media browser to select videos.

## Signature

```csharp
System.Threading.Tasks.Task<System.Collections.Generic.List<Microsoft.Maui.Storage.FileResult!>!>! static PickVideosAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Remarks

On Android, not all picker user interfaces enforce the `SelectionLimit`. On Windows, `SelectionLimit` is not supported. Implement your own logic to ensure that the limit is maintained and/or notify the user on these platforms.

## Returns

A list of `FileResult` objects containing details of the picked videos. When the operation was cancelled by the user, this will return an empty list.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[MediaPicker|MediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
