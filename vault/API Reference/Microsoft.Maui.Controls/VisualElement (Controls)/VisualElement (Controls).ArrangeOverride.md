---
title: "VisualElement (Controls).ArrangeOverride"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.ArrangeOverride"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).ArrangeOverride

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Allows subclasses to implement custom Arrange logic during a controls layout pass.

## Signature

```csharp
Microsoft.Maui.Graphics.Size virtual ArrangeOverride(Microsoft.Maui.Graphics.Rect bounds)
```

## Parameters

| Parameter | Description |
|---|---|
| `bounds` | The new bounds of the element. |

## Returns

The resulting size of this element's frame by the platform.

## Remarks

Subclasses will still want to call `ArrangeOverride` on the base class or call `PlatformArrange` on the `Handler` .

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
