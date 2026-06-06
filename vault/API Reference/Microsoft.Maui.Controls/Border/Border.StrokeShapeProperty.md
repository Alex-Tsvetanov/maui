---
title: "Border.StrokeShapeProperty"
tags:
  - api
  - member/field
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Border.StrokeShapeProperty"
declaring_type: "Border"
member_kind: field
---

# Border.StrokeShapeProperty

> [!abstract] Field of [[Border|Border]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the child content that is placed inside the border. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Controls.BindableProperty! static readonly StrokeShapeProperty
```

## Remarks

This property controls which edges of the border should obey safe area insets. Use SafeAreaRegions.None for edge-to-edge content, SafeAreaRegions.All to obey all safe area insets, SafeAreaRegions.Container for content that flows under keyboard but stays out of bars/notch, or SafeAreaRegions.Keyboard for keyboard-aware behavior.

## See also

- Declaring type: [[Border|Border]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
