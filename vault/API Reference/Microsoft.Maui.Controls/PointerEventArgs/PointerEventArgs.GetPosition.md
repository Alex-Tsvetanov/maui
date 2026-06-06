---
title: "PointerEventArgs.GetPosition"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PointerEventArgs.GetPosition"
declaring_type: "PointerEventArgs"
member_kind: method
---

# PointerEventArgs.GetPosition

> [!abstract] Method of [[PointerEventArgs|PointerEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

When overridden in a derived class, gets the position of the pointer.

## Signature

```csharp
Microsoft.Maui.Graphics.Point? virtual GetPosition(Microsoft.Maui.Controls.Element? relativeTo)
```

## Parameters

| Parameter | Description |
|---|---|
| `relativeTo` | Where the pointer will be measured from. |

## Returns

The position relative to the `Element`.

## Remarks

Gets the position of the pointer relative to the element by default.

## See also

- Declaring type: [[PointerEventArgs|PointerEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
