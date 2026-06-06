---
title: "IWindowStateManager.OnActivated"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IWindowStateManager.OnActivated"
declaring_type: "IWindowStateManager"
member_kind: method
---

# IWindowStateManager.OnActivated

> [!abstract] Method of [[IWindowStateManager|IWindowStateManager]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Sets the new active window that can be retrieved with `GetActiveWindow`.

## Signature

```csharp
void OnActivated(Microsoft.UI.Xaml.Window! window, Microsoft.UI.Xaml.WindowActivatedEventArgs! args)
```

## Parameters

| Parameter | Description |
|---|---|
| `window` | The `Window` object that is activated. |
| `args` | The associated event arguments for this window activation event. |

## See also

- Declaring type: [[IWindowStateManager|IWindowStateManager]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
