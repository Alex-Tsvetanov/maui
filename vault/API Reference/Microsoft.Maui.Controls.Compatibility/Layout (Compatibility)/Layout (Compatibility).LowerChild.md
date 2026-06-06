---
title: "Layout (Compatibility).LowerChild"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout.LowerChild"
declaring_type: "Layout (Compatibility)"
member_kind: method
---

# Layout (Compatibility).LowerChild

> [!abstract] Method of [[Layout (Compatibility)|Layout (Compatibility)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

Sends a child to the back of the visual stack.

## Signature

```csharp
void LowerChild(Microsoft.Maui.Controls.View view)
```

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view to lower in the visual stack. |

## Remarks

Children are internally stored in visual stack order. This means that raising or lowering a child also changes the order in which the children are enumerated.

## See also

- Declaring type: [[Layout (Compatibility)|Layout (Compatibility)]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
