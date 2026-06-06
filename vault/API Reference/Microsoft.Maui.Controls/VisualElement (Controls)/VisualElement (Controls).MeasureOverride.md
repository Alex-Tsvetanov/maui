---
title: "VisualElement (Controls).MeasureOverride"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.MeasureOverride"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).MeasureOverride

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Allows subclasses to implement custom Measure logic during a controls measure pass.

## Signature

```csharp
Microsoft.Maui.Graphics.Size virtual MeasureOverride(double widthConstraint, double heightConstraint)
```

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The width constraint to request. |
| `heightConstraint` | The height constraint to request. |

## Returns

The requested size that an element wants in order to be displayed on the device.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
