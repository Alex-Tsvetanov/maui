---
title: "View.VerticalOptions"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.View.VerticalOptions"
declaring_type: "View"
member_kind: property
---

# View.VerticalOptions

> [!abstract] Property of [[View|View]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the `LayoutOptions` that define how the element gets arrange in a layout cycle. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Controls.LayoutOptions VerticalOptions { get; set; }
```

## Remarks

Assigning `VerticalOptions` modifies how the element is arrange when there is excess space available along the Y axis from the parent layout. If multiple elements inside a layout are set to expand, the extra space is distributed proportionally.

## See also

- Declaring type: [[View|View]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
