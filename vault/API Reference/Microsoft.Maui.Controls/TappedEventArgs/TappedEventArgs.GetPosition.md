---
title: "TappedEventArgs.GetPosition"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TappedEventArgs.GetPosition"
declaring_type: "TappedEventArgs"
member_kind: method
---

# TappedEventArgs.GetPosition

> [!abstract] Method of [[TappedEventArgs|TappedEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Gets the position of the tap relative to the specified element.

## Signature

```csharp
Microsoft.Maui.Graphics.Point? virtual GetPosition(Microsoft.Maui.Controls.Element? relativeTo)
```

## Parameters

| Parameter | Description |
|---|---|
| `relativeTo` | The element to use as the coordinate reference, or `null` for screen coordinates. |

## Returns

The tap position, or `null` if not available.

## See also

- Declaring type: [[TappedEventArgs|TappedEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
