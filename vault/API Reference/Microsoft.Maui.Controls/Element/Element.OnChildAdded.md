---
title: "Element.OnChildAdded"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Element.OnChildAdded"
declaring_type: "Element"
member_kind: method
---

# Element.OnChildAdded

> [!abstract] Method of [[Element|Element]]
> Namespace: `Microsoft.Maui.Controls`

Raises the `ChildAdded` event. Implement this method to add class handling for this event.

## Signature

```csharp
void virtual OnChildAdded(Microsoft.Maui.Controls.Element child)
```

## Parameters

| Parameter | Description |
|---|---|
| `child` | The element that's been added as a child. |

## Remarks

This method has no default implementation. You should still call the base implementation in case an intermediate class has implemented this method.

## See also

- Declaring type: [[Element|Element]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
