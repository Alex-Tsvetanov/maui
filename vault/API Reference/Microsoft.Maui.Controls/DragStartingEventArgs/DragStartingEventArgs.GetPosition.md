---
title: "DragStartingEventArgs.GetPosition"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.DragStartingEventArgs.GetPosition"
declaring_type: "DragStartingEventArgs"
member_kind: method
---

# DragStartingEventArgs.GetPosition

> [!abstract] Method of [[DragStartingEventArgs|DragStartingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets the location where dragging started relative to the specified element.

## Signature

```csharp
Microsoft.Maui.Graphics.Point? virtual GetPosition(Microsoft.Maui.Controls.Element? relativeTo)
```

## Parameters

| Parameter | Description |
|---|---|
| `relativeTo` | Element whose position is used to calculate the relative position. |

## Returns

The point where dragging started relative to `relativeTo`.

## Remarks

If `relativeTo` is `null` then the position relative to the screen is returned.

## See also

- Declaring type: [[DragStartingEventArgs|DragStartingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
