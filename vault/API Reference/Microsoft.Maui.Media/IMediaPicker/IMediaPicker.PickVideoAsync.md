---
title: "IMediaPicker.PickVideoAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.IMediaPicker.PickVideoAsync"
declaring_type: "IMediaPicker"
member_kind: method
---

# IMediaPicker.PickVideoAsync

> [!abstract] Method of [[IMediaPicker|IMediaPicker]]
> Namespace: `Microsoft.Maui.Media`

Opens the media browser to select a video.

## Signature

```csharp
System.Threading.Tasks.Task<Microsoft.Maui.Storage.FileResult?>! PickVideoAsync(Microsoft.Maui.Media.MediaPickerOptions? options = null)
```

## Remarks

When using `SelectionLimit` on this overload, it will not have effect.

## Returns

A `FileResult` object containing details of the picked video. When the operation was cancelled by the user, this will return `null`.

## Parameters

| Parameter | Description |
|---|---|
| `options` | Pick options to use. |

## See also

- Declaring type: [[IMediaPicker|IMediaPicker]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
