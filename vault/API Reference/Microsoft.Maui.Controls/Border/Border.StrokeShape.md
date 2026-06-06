---
title: "Border.StrokeShape"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Border.StrokeShape"
declaring_type: "Border"
member_kind: property
---

# Border.StrokeShape

> [!abstract] Property of [[Border|Border]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the shape of the border. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Graphics.IShape? StrokeShape { get; set; }
```

## Remarks

The default value is a `Rectangle`. You can set this to other shapes like `RoundRectangle`, `Ellipse`, or any custom `IShape` implementation to change the border's appearance.

## See also

- Declaring type: [[Border|Border]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
