---
title: "VisualElement (Controls).SizeAllocated"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.SizeAllocated"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).SizeAllocated

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Method that is called during a layout cycle to signal the start of a sub-tree layout.

## Signature

```csharp
void SizeAllocated(double width, double height)
```

## Parameters

| Parameter | Description |
|---|---|
| `width` | The newly allocated width. |
| `height` | The newly allocated height. |

## Remarks

Calling `SizeAllocated` will start a new layout cycle on the children of the element. Excessive calls to this method may cause performance problems.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
