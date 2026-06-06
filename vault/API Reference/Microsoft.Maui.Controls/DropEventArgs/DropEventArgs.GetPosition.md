---
title: "DropEventArgs.GetPosition"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.DropEventArgs.GetPosition"
declaring_type: "DropEventArgs"
member_kind: method
---

# DropEventArgs.GetPosition

> [!abstract] Method of [[DropEventArgs|DropEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets the location of the drag relative to the specified element.

## Signature

```csharp
Microsoft.Maui.Graphics.Point? virtual GetPosition(Microsoft.Maui.Controls.Element? relativeTo)
```

## Parameters

| Parameter | Description |
|---|---|
| `relativeTo` | Element whose position is used to calculate the relative position. |

## Returns

The point where dragging is occurring relative to `relativeTo`.

## Remarks

If `relativeTo` is `null` then the position relative to the screen is returned.

## See also

- Declaring type: [[DropEventArgs|DropEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
