---
title: "VisualElement (Controls).Focus"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Focus"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).Focus

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Attempts to set focus to this element.

## Signature

```csharp
bool Focus()
```

## Returns

`true` if the keyboard focus was set to this element; if the call to this method did not force a focus change.

## Remarks

Element must be able to receive focus for this to work. Calling `Focus` on offscreen or unrealized elements has undefined behavior.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
