---
title: "Image (Controls).IsOpaque"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Image.IsOpaque"
declaring_type: "Image (Controls)"
member_kind: property
---

# Image (Controls).IsOpaque

> [!abstract] Property of [[Image (Controls)|Image (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a Boolean value that hints to the rendering engine that it may safely omit drawing visual elements behind the image. This is a bindable property.

## Signature

```csharp
bool IsOpaque { get; set; }
```

## Remarks

Setting this property does not change the visual opacity of the image. Instead, it provides a hint to the rendering engine that may improve performance by skipping the rendering of elements behind the image.

## See also

- Declaring type: [[Image (Controls)|Image (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
