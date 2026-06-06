---
title: "View.HorizontalOptions"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.View.HorizontalOptions"
declaring_type: "View"
member_kind: property
---

# View.HorizontalOptions

> [!abstract] Property of [[View|View]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the `LayoutOptions` that define how the element gets arranged in a layout cycle. This is a bindable property.

## Signature

```csharp
Microsoft.Maui.Controls.LayoutOptions HorizontalOptions { get; set; }
```

## Remarks

Assigning `HorizontalOptions` modifies how the element is arranged when there is excess space available along the X axis from the parent layout. If multiple elements inside a layout are set to expand, the extra space is distributed proportionally.

## See also

- Declaring type: [[View|View]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
