---
title: "MediaPickerOptions.CompressionQuality"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.MediaPickerOptions.CompressionQuality"
declaring_type: "MediaPickerOptions"
member_kind: property
---

# MediaPickerOptions.CompressionQuality

> [!abstract] Property of [[MediaPickerOptions|MediaPickerOptions]]
> Namespace: `Microsoft.Maui.Media`

Gets or sets the compression quality for picked media. The value should be between 0 and 100, where 0 is the lowest quality (most compression) and 100 is the highest quality (least compression).

## Signature

```csharp
int CompressionQuality { get; set; }
```

## Remarks

Please note that performance might be affected by the compression quality, especially on lower-end devices. For JPEG images, this controls the lossy compression quality directly. For PNG images, values below 90 will convert to JPEG format for better compression. Values 90-99 will scale down the PNG image. Value 100 preserves original PNG format and quality.

## See also

- Declaring type: [[MediaPickerOptions|MediaPickerOptions]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
