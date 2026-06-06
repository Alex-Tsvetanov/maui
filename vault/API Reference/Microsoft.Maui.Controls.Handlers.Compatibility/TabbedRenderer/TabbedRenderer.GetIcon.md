---
title: "TabbedRenderer.GetIcon"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Handlers-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Handlers.Compatibility.TabbedRenderer.GetIcon"
declaring_type: "TabbedRenderer"
member_kind: method
---

# TabbedRenderer.GetIcon

> [!abstract] Method of [[TabbedRenderer|TabbedRenderer]]
> Namespace: `Microsoft.Maui.Controls.Handlers.Compatibility`

Get the icon for the tab bar item of this page

## Signature

```csharp
System.Threading.Tasks.Task<System.Tuple<UIKit.UIImage, UIKit.UIImage>> virtual GetIcon(Microsoft.Maui.Controls.Page page)
```

## Returns

A tuple containing as item1: the unselected version of the icon, item2: the selected version of the icon (item2 can be null), or null if no icon should be set.

## See also

- Declaring type: [[TabbedRenderer|TabbedRenderer]]
- [[_Microsoft.Maui.Controls.Handlers.Compatibility|Microsoft.Maui.Controls.Handlers.Compatibility namespace]]
