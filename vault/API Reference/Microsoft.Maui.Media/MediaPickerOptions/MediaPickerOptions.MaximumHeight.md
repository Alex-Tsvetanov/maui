---
title: "MediaPickerOptions.MaximumHeight"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPickerOptions.MaximumHeight"
declaring_type: "MediaPickerOptions"
member_kind: property
---

# MediaPickerOptions.MaximumHeight

> [!abstract] Property of [[MediaPickerOptions|MediaPickerOptions]]
> Namespace: `Microsoft.Maui.Media`

Gets or sets the maximum height for image resizing. When set, images will be resized to fit within this height while preserving aspect ratio. A value of 0 or null means no height constraint.

## Signature

```csharp
int? MaximumHeight { get; set; }
```

## Remarks

This property only applies to images. It has no effect on video files. The image will be resized to fit within the specified maximum dimensions while maintaining aspect ratio. If both MaximumWidth and MaximumHeight are specified, the image will be scaled to fit within both constraints. This resizing is applied before any compression quality settings.

## See also

- Declaring type: [[MediaPickerOptions|MediaPickerOptions]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
