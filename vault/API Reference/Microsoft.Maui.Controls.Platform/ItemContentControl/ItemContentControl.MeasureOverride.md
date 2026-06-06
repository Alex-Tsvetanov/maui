---
title: "ItemContentControl.MeasureOverride"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.ItemContentControl.MeasureOverride"
declaring_type: "ItemContentControl"
member_kind: method
---

# ItemContentControl.MeasureOverride

> [!abstract] Method of [[ItemContentControl|ItemContentControl]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Allows subclasses to implement custom Measure logic during a controls measure pass.

## Signature

```csharp
Windows.Foundation.Size override MeasureOverride(Windows.Foundation.Size availableSize)
```

## Returns

The requested size that an element wants in order to be displayed on the device.

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The width constraint to request. |
| `heightConstraint` | The height constraint to request. |

## See also

- Declaring type: [[ItemContentControl|ItemContentControl]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
