---
title: "Element.OnParentChanging"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Element.OnParentChanging"
declaring_type: "Element"
member_kind: method
---

# Element.OnParentChanging

> [!abstract] Method of [[Element|Element]]
> Namespace: `Microsoft.Maui.Controls`

When overridden in a derived class, should raise the `ParentChanging` event.

## Signature

```csharp
void virtual OnParentChanging(Microsoft.Maui.Controls.ParentChangingEventArgs args)
```

## Parameters

| Parameter | Description |
|---|---|
| `args` | Provides data for the `ParentChanging` event. |

## Remarks

It is the implementor's responsibility to raise the `ParentChanging` event.

## See also

- Declaring type: [[Element|Element]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
