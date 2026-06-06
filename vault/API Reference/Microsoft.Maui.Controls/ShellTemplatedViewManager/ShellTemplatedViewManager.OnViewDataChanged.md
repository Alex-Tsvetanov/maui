---
title: "ShellTemplatedViewManager.OnViewDataChanged"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ShellTemplatedViewManager.OnViewDataChanged"
declaring_type: "ShellTemplatedViewManager"
member_kind: method
---

# ShellTemplatedViewManager.OnViewDataChanged

> [!abstract] Method of [[ShellTemplatedViewManager|ShellTemplatedViewManager]]
> Namespace: `Microsoft.Maui.Controls`

Updates the local view reference when the bound view data changes, invoking the supplied child add and remove callbacks.

## Signature

```csharp
void static OnViewDataChanged(Microsoft.Maui.Controls.DataTemplate currentViewTemplate, ref Microsoft.Maui.Controls.View localViewRef, object newViewData, System.Action<Microsoft.Maui.Controls.Element> OnChildRemoved, System.Action<Microsoft.Maui.Controls.Element> OnChildAdded)
```

## See also

- Declaring type: [[ShellTemplatedViewManager|ShellTemplatedViewManager]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
