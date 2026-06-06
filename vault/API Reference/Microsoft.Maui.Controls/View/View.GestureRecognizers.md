---
title: "View.GestureRecognizers"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.View.GestureRecognizers"
declaring_type: "View"
member_kind: property
---

# View.GestureRecognizers

> [!abstract] Property of [[View|View]]
> Namespace: `Microsoft.Maui.Controls`

The collection of gesture recognizers associated with this view.

## Signature

```csharp
System.Collections.Generic.IList<Microsoft.Maui.Controls.IGestureRecognizer> GestureRecognizers { get; }
```

## Remarks

Adding items to this collection will associate gesture events with this element. It is not recommended to add gesture recognizers for gestures that elements already natively support. For example, adding a `TapGestureRecognizer` to a `Button` may lead to unexpected results.

## See also

- Declaring type: [[View|View]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
