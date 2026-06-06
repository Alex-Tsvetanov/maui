---
title: "ItemContentControl.ArrangeOverride"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Platform
aliases:
  - "Microsoft.Maui.Controls.Platform.ItemContentControl.ArrangeOverride"
declaring_type: "ItemContentControl"
member_kind: method
---

# ItemContentControl.ArrangeOverride

> [!abstract] Method of [[ItemContentControl|ItemContentControl]]
> Namespace: `Microsoft.Maui.Controls.Platform`

Allows subclasses to implement custom Arrange logic during a controls layout pass.

## Signature

```csharp
Windows.Foundation.Size override ArrangeOverride(Windows.Foundation.Size finalSize)
```

## Remarks

Subclasses will still want to call `ArrangeOverride` on the base class or call `PlatformArrange` on the `Handler` .

## Returns

The resulting size of this element's frame by the platform.

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The new bounds of the element. |

## See also

- Declaring type: [[ItemContentControl|ItemContentControl]]
- [[_Microsoft.Maui.Controls.Platform|Microsoft.Maui.Controls.Platform namespace]]
