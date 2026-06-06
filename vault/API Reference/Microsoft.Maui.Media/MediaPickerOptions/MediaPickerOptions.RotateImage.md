---
title: "MediaPickerOptions.RotateImage"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPickerOptions.RotateImage"
declaring_type: "MediaPickerOptions"
member_kind: property
---

# MediaPickerOptions.RotateImage

> [!abstract] Property of [[MediaPickerOptions|MediaPickerOptions]]
> Namespace: `Microsoft.Maui.Media`

Gets or sets whether to automatically rotate the image based on EXIF orientation data. When true, the image will be rotated to the correct orientation. Default value is false.

## Signature

```csharp
bool RotateImage { get; set; }
```

## Remarks

This property only applies to images. It has no effect on video files. When enabled, the EXIF orientation data will be applied to correctly orient the image, and the orientation flag will be reset to avoid duplicate rotations in image viewers. This rotation happens before any resizing or compression is applied. Please note that performance might be affected by the rotation operation, especially on lower-end devices.

## See also

- Declaring type: [[MediaPickerOptions|MediaPickerOptions]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
