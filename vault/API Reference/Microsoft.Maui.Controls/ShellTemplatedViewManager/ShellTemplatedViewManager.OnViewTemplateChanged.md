---
title: "ShellTemplatedViewManager.OnViewTemplateChanged"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ShellTemplatedViewManager.OnViewTemplateChanged"
declaring_type: "ShellTemplatedViewManager"
member_kind: method
---

# ShellTemplatedViewManager.OnViewTemplateChanged

> [!abstract] Method of [[ShellTemplatedViewManager|ShellTemplatedViewManager]]
> Namespace: `Microsoft.Maui.Controls`

Updates the local view reference when the view template changes, invoking the supplied child add and remove callbacks.

## Signature

```csharp
void static OnViewTemplateChanged(Microsoft.Maui.Controls.DataTemplate newViewTemplate, ref Microsoft.Maui.Controls.View localViewRef, object currentViewData, System.Action<Microsoft.Maui.Controls.Element> OnChildRemoved, System.Action<Microsoft.Maui.Controls.Element> OnChildAdded, Microsoft.Maui.Controls.Shell shell)
```

## See also

- Declaring type: [[ShellTemplatedViewManager|ShellTemplatedViewManager]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
