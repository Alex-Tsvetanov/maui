---
title: "VisualElement (Controls).InputTransparent"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.InputTransparent"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).InputTransparent

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value indicating whether this element responds to hit testing during user interaction. This is a bindable property.

## Signature

```csharp
bool InputTransparent { get; set; }
```

## Remarks

The default value is `false`. Setting `InputTransparent` to `true` makes the element invisible to touch and pointer input. The input is passed to the first non-input-transparent element that is visually behind the input transparent element.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
