---
title: "VisualElement (Controls).Measure"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Measure"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).Measure

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Returns the minimum size that an element needs in order to be displayed on the device. Margins are excluded from the measurement, but returned with the size. It is not recommended to call this method outside of the `MeasureOverride` pass on the parent element.

## Signatures

```csharp
Microsoft.Maui.Graphics.Size Measure(double widthConstraint, double heightConstraint)
Microsoft.Maui.SizeRequest virtual Measure(double widthConstraint, double heightConstraint, Microsoft.Maui.Controls.MeasureFlags flags = Microsoft.Maui.Controls.MeasureFlags.None)
```

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The suggested maximum width constraint for the element to render. |
| `heightConstraint` | The suggested maximum height constraint for the element to render. |

## Returns

The minimum size that an element needs in order to be displayed on the device.

## Remarks

If the minimum size that the element needs in order to be displayed on the device is larger than can be accommodated by `widthConstraint` and `heightConstraint`, the return value may represent a rectangle that is larger in either one or both of those parameters.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
