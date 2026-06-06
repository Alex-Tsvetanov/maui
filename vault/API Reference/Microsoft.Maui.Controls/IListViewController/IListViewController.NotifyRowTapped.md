---
title: "IListViewController.NotifyRowTapped"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.IListViewController.NotifyRowTapped"
declaring_type: "IListViewController"
member_kind: method
---

# IListViewController.NotifyRowTapped

> [!abstract] Method of [[IListViewController|IListViewController]]
> Namespace: `Microsoft.Maui.Controls`

Notifies the list view that the specified row was tapped, optionally indicating a context-menu request.

## Signatures

```csharp
void NotifyRowTapped(int index, int inGroupIndex, Microsoft.Maui.Controls.Cell cell, bool isContextMenuRequested)
void NotifyRowTapped(int index, int inGroupIndex, Microsoft.Maui.Controls.Cell cell)
void NotifyRowTapped(int index, Microsoft.Maui.Controls.Cell cell, bool isContextMenuRequested)
void NotifyRowTapped(int index, Microsoft.Maui.Controls.Cell cell)
```

## See also

- Declaring type: [[IListViewController|IListViewController]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
