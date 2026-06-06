---
title: "Element.OnChildRemoved"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Element.OnChildRemoved"
declaring_type: "Element"
member_kind: method
---

# Element.OnChildRemoved

> [!abstract] Method of [[Element|Element]]
> Namespace: `Microsoft.Maui.Controls`

Raises the `ChildRemoved` event. Implement this method to add class handling for this event

## Signature

```csharp
void virtual OnChildRemoved(Microsoft.Maui.Controls.Element child, int oldLogicalIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `child` | The child element that's been removed. |
| `oldLogicalIndex` | The child's element index in the logical tree. |

## Remarks

This method has no default implementation. You should still call the base implementation in case an intermediate class has implemented this method. If not debugging, the logical tree index will not have any effect.

## See also

- Declaring type: [[Element|Element]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
