---
title: "VisualStateManager.HasVisualStateGroups"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualStateManager.HasVisualStateGroups"
declaring_type: "VisualStateManager"
member_kind: method
---

# VisualStateManager.HasVisualStateGroups

> [!abstract] Method of [[VisualStateManager|VisualStateManager]]
> Namespace: `Microsoft.Maui.Controls`

Returns `true` for states that the MAUI framework drives automatically (Disabled, Focused, Unfocused, Selected, PointerOver, Pressed). Only these states promote an implicit-style VSM setter to full VSM priority (fix for #34363), preventing custom developer-defined states from unexpectedly overriding manually-set values.

## Signature

```csharp
bool static HasVisualStateGroups(this Microsoft.Maui.Controls.VisualElement element)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The visual element to check. |

## Returns

`true` if the element has visual state groups; otherwise, `false`.

## See also

- Declaring type: [[VisualStateManager|VisualStateManager]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
