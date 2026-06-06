---
title: "Border.CrossPlatformArrange"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Border.CrossPlatformArrange"
declaring_type: "Border"
member_kind: method
---

# Border.CrossPlatformArrange

> [!abstract] Method of [[Border|Border]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the shape of the border. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Graphics.Size CrossPlatformArrange(Microsoft.Maui.Graphics.Rect bounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The available bounds for the border. |

## Returns

The actual size used by the border.

## Remarks

The default value is a `Rectangle`. You can set this to other shapes like `RoundRectangle`, `Ellipse`, or any custom `IShape` implementation to change the border's appearance.

## See also

- Declaring type: [[Border|Border]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
