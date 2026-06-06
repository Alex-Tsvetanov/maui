---
title: "ViewExtensions (Controls).InvalidateMeasure"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ViewExtensions.InvalidateMeasure"
declaring_type: "ViewExtensions (Controls)"
member_kind: method
---

# ViewExtensions (Controls).InvalidateMeasure

> [!abstract] Method of [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Layout updates can be forced by app code rather than relying on the built-in layout system behavior. However, that is not generally recommended. Calling InvalidateArrange, InvalidateMeasure or UpdateLayout is usually unnecessary and can cause poor performance if overused. In many situations where app code might be changing layout properties, the layout system will probably already be processing updates asynchronously. The layout system also has optimizations for dealing with cascades of layout changes through parent-child relationships, and forcing layout with app code can work against such optimizations. Nevertheless, it's possible that layout situations exist in more complicated scenarios where forcing layout is the best option for resolving a timing issue or other issue with layout. Just use it deliberately and sparingly.

## Signature

```csharp
void static InvalidateMeasure(this Microsoft.Maui.Controls.VisualElement! view)
```

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view on which this method operates. |

## See also

- Declaring type: [[ViewExtensions (Controls)|ViewExtensions (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
