---
title: "PickOptions.FileTypes"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Storage
aliases:
  - "Microsoft.Maui.Storage.PickOptions.FileTypes"
declaring_type: "PickOptions"
member_kind: property
---

# PickOptions.FileTypes

> [!abstract] Property of [[PickOptions|PickOptions]]
> Namespace: `Microsoft.Maui.Storage`

List of file types that file file picker should return.

## Signature

```csharp
Microsoft.Maui.Storage.FilePickerFileType? FileTypes { get; set; }
```

## Remarks

On Android and iOS the files not matching this list are shown in the file picker, but will be grayed out and cannot be selected. When the `Value` array is `null` or empty, all file types can be selected while picking. The contents of this array is platform specific; every platform has its own way to specify the file types. On Android you can specify one or more MIME types, e.g. image/png . Additionally, wildcard characters can be used, e.g. image/* On iOS, you can specify UTType constants, e.g. UTType.Image . On Windows, you can specify a list of extensions, like this: ".jpg", ".png" .

## See also

- Declaring type: [[PickOptions|PickOptions]]
- [[_Microsoft.Maui.Storage|Microsoft.Maui.Storage namespace]]
