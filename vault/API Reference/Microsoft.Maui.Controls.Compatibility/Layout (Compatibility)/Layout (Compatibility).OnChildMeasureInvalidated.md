---
title: "Layout (Compatibility).OnChildMeasureInvalidated"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout.OnChildMeasureInvalidated"
declaring_type: "Layout (Compatibility)"
member_kind: method
---

# Layout (Compatibility).OnChildMeasureInvalidated

> [!abstract] Method of [[Layout (Compatibility)|Layout (Compatibility)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

Invoked whenever a child of the layout has emitted `MeasureInvalidated`. Implement this method to add class handling for this event.

## Signatures

```csharp
void OnChildMeasureInvalidated(object sender, System.EventArgs e)
void virtual OnChildMeasureInvalidated()
```

## Parameters

| Parameter | Description |
|---|---|
| `sender` | The child element whose preferred size changed. |
| `e` | The event data. |

## Remarks

This method has a default implementation and application developers must call the base implementation.

## See also

- Declaring type: [[Layout (Compatibility)|Layout (Compatibility)]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
