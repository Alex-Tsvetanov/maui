---
title: "TemplatedView.LowerChild"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TemplatedView.LowerChild"
declaring_type: "TemplatedView"
member_kind: method
---

# TemplatedView.LowerChild

> [!abstract] Method of [[TemplatedView|TemplatedView]]
> Namespace: `Microsoft.Maui.Controls`

Sends a child to the back of the visual stack.

## Signature

```csharp
void LowerChild(Microsoft.Maui.Controls.View! view)
```

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view to lower in the visual stack. |

## Remarks

Children are internally stored in visual stack order. This means that raising or lowering a child also changes the order in which the children are enumerated.

## See also

- Declaring type: [[TemplatedView|TemplatedView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
