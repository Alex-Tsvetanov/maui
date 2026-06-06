---
title: "MediaPickerOptions.PreserveMetaData"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPickerOptions.PreserveMetaData"
declaring_type: "MediaPickerOptions"
member_kind: property
---

# MediaPickerOptions.PreserveMetaData

> [!abstract] Property of [[MediaPickerOptions|MediaPickerOptions]]
> Namespace: `Microsoft.Maui.Media`

Gets or sets whether to preserve metadata (including EXIF data) when processing images. When true, metadata from the original image will be preserved in the processed image. Default value is true.

## Signature

```csharp
bool PreserveMetaData { get; set; }
```

## Remarks

This property only applies to images. It has no effect on video files. When enabled, metadata such as EXIF data, GPS information, camera settings, and timestamps will be copied from the original image to the processed image during operations like resizing, compression, or rotation. Setting this to false may result in smaller file sizes but will lose the image's metadata. Currently not supported on Windows.

## See also

- Declaring type: [[MediaPickerOptions|MediaPickerOptions]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
