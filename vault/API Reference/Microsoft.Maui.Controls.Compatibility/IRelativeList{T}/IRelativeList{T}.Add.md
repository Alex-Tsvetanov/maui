---
title: "IRelativeList<T>.Add"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.RelativeLayout.IRelativeList<T>.Add"
declaring_type: "IRelativeList<T>"
member_kind: method
---

# IRelativeList<T>.Add

> [!abstract] Method of [[IRelativeList{T}|IRelativeList<T>]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

Adds a view to the relative layout using the specified positioning constraints.

## Signatures

```csharp
void Microsoft.Maui.Controls.Compatibility.RelativeLayout.IRelativeList<T>.Add(T view, Microsoft.Maui.Controls.Compatibility.Constraint xConstraint = null, Microsoft.Maui.Controls.Compatibility.Constraint yConstraint = null, Microsoft.Maui.Controls.Compatibility.Constraint widthConstraint = null, Microsoft.Maui.Controls.Compatibility.Constraint heightConstraint = null)
void Microsoft.Maui.Controls.Compatibility.RelativeLayout.IRelativeList<T>.Add(T view, System.Linq.Expressions.Expression<System.Func<double>> x = null, System.Linq.Expressions.Expression<System.Func<double>> y = null, System.Linq.Expressions.Expression<System.Func<double>> width = null, System.Linq.Expressions.Expression<System.Func<double>> height = null)
void Microsoft.Maui.Controls.Compatibility.RelativeLayout.IRelativeList<T>.Add(T view, System.Linq.Expressions.Expression<System.Func<Microsoft.Maui.Graphics.Rect>> bounds)
```

## See also

- Declaring type: [[IRelativeList{T}|IRelativeList<T>]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
