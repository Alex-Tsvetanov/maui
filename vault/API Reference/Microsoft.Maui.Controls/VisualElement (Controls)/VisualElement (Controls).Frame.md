---
title: "VisualElement (Controls).Frame"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Frame"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).Frame

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the frame this element resides in on screen.

## Signature

```csharp
Microsoft.Maui.Graphics.Rect Frame { get; set; }
```

## Remarks

Setting this property outside of `ArrangeOverride` won't do anything. If you want to influence this property you'll need to override `ArrangeOverride`

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
