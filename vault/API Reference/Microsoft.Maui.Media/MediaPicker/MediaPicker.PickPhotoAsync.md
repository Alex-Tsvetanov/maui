---
title: "MediaPicker.PickPhotoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPicker.PickPhotoAsync"
declaring_type: "MediaPicker"
member_kind: method
---

# MediaPicker.PickPhotoAsync

> [!abstract] Method of [[MediaPicker|MediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the media browser to select a photo.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! static PickPhotoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Remarks

When using `SelectionLimit` on this overload, it will not have effect.

## Returns

A `FileResult` object containing details of the picked photo. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[MediaPicker|MediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
