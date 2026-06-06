---
title: "AutomationProperties.GetLabeledBy"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AutomationProperties.GetLabeledBy"
declaring_type: "AutomationProperties"
member_kind: method
---

# AutomationProperties.GetLabeledBy

> [!abstract] Method of [[AutomationProperties|AutomationProperties]]
> Namespace: `Microsoft.Maui.Controls`

Returns the element that labels `bindable`, if `bindable` does not label itself and if another element describes it in the UI.

## Signature

```csharp
Microsoft.Maui.Controls.VisualElement static GetLabeledBy(Microsoft.Maui.Controls.BindableObject bindable)
```

## Parameters

| Parameter | Description |
|---|---|
| `bindable` | The object whose label to find. |

## Returns

The element that labels `bindable`, if present.

## See also

- Declaring type: [[AutomationProperties|AutomationProperties]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
