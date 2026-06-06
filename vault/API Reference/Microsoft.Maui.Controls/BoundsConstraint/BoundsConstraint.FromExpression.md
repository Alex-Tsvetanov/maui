---
title: "BoundsConstraint.FromExpression"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BoundsConstraint.FromExpression"
declaring_type: "BoundsConstraint"
member_kind: method
---

# BoundsConstraint.FromExpression

> [!abstract] Method of [[BoundsConstraint|BoundsConstraint]]
> Namespace: `Microsoft.Maui.Controls`

Returns a `BoundsConstraint` object that contains the compiled version of `expression` and is relative to either `parents` or the views referred to in `expression`.

## Signature

```csharp
Microsoft.Maui.Controls.BoundsConstraint static FromExpression(System.Linq.Expressions.Expression<System.Func<Microsoft.Maui.Graphics.Rect>> expression, System.Collections.Generic.IEnumerable<Microsoft.Maui.Controls.View> parents = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `expression` | The expression from which to compile the constraint. |
| `parents` | The parents to consider when compiling the constraint. |

## See also

- Declaring type: [[BoundsConstraint|BoundsConstraint]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
