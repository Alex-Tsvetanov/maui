---
title: "CellRenderer.GetCell"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Handlers-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Handlers.Compatibility.CellRenderer.GetCell"
declaring_type: "CellRenderer"
member_kind: method
---

# CellRenderer.GetCell

> [!abstract] Method of [[CellRenderer|CellRenderer]]
> Namespace: `Microsoft.Maui.Controls.Handlers.Compatibility`

Returns the native cell view for the specified cross-platform cell, reusing an existing view when available.

## Signatures

```csharp
Android.Views.View GetCell(Microsoft.Maui.Controls.Cell item, Android.Views.View convertView, Android.Views.ViewGroup parent, Android.Content.Context context)
UIKit.UITableViewCell! virtual GetCell(Microsoft.Maui.Controls.Cell! item, UIKit.UITableViewCell! reusableCell, UIKit.UITableView! tv)
```

## See also

- Declaring type: [[CellRenderer|CellRenderer]]
- [[_Microsoft.Maui.Controls.Handlers.Compatibility|Microsoft.Maui.Controls.Handlers.Compatibility namespace]]
