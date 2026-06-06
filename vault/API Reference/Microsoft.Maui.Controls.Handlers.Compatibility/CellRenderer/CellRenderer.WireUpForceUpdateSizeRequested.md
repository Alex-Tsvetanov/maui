---
title: "CellRenderer.WireUpForceUpdateSizeRequested"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Handlers-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Handlers.Compatibility.CellRenderer.WireUpForceUpdateSizeRequested"
declaring_type: "CellRenderer"
member_kind: method
---

# CellRenderer.WireUpForceUpdateSizeRequested

> [!abstract] Method of [[CellRenderer|CellRenderer]]
> Namespace: `Microsoft.Maui.Controls.Handlers.Compatibility`

Wires up handling so the native cell is resized when the cross-platform cell requests a forced size update.

## Signatures

```csharp
void WireUpForceUpdateSizeRequested(Microsoft.Maui.Controls.Cell cell, Android.Views.View platformCell)
void WireUpForceUpdateSizeRequested(Microsoft.Maui.Controls.ICellController! cell, UIKit.UITableViewCell! platformCell, UIKit.UITableView! tableView)
```

## See also

- Declaring type: [[CellRenderer|CellRenderer]]
- [[_Microsoft.Maui.Controls.Handlers.Compatibility|Microsoft.Maui.Controls.Handlers.Compatibility namespace]]
