---
title: "Element.FindByName"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Element.FindByName"
declaring_type: "Element"
member_kind: method
---

# Element.FindByName

> [!abstract] Method of [[Element|Element]]
> Namespace: `Microsoft.Maui.Controls`

Returns the element that has the specified name.

## Signature

```csharp
object FindByName(string name)
```

## Parameters

| Parameter | Description |
|---|---|
| `name` | The name of the element to be found. |

## Returns

The element that has the specified name, or `null` if no element with the specified name is found.

## Remarks

This method searches for named elements within the current namescope. The search scope is determined by traversing up the visual tree from the current element until a namescope is found. Typically, each page, content view, or data template defines its own namescope. The search is limited to elements that have been registered in the same namescope, which includes: Elements with x:Name attributes defined in XAML within the same namescope Elements manually registered using `RegisterName` Child elements and their descendants within the same namescope boundary Elements in different namescopes (such as different pages or data templates) are not accessible from each other through this method.

## See also

- Declaring type: [[Element|Element]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
