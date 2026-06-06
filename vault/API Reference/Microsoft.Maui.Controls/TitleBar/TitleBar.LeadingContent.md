---
title: "TitleBar.LeadingContent"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TitleBar.LeadingContent"
declaring_type: "TitleBar"
member_kind: property
---

# TitleBar.LeadingContent

> [!abstract] Property of [[TitleBar|TitleBar]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a `View` control that represents the leading content. The leading content follows the optional `Icon` and is aligned to the left or right of the title bar, depending on the `FlowDirection`. Views set here will be allocated as much space as they require. Views set here will block all input to the title bar region and handle input directly.

## Signature

```csharp
Microsoft.Maui.IView? LeadingContent { get; set; }
```

## See also

- Declaring type: [[TitleBar|TitleBar]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
